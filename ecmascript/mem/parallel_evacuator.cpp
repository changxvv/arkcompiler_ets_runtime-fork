/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ecmascript/mem/parallel_evacuator-inl.h"

#include "ecmascript/js_hclass-inl.h"
#include "ecmascript/mem/barriers-inl.h"
#include "ecmascript/mem/clock_scope.h"
#include "ecmascript/mem/concurrent_sweeper.h"
#include "ecmascript/mem/gc_bitset.h"
#include "ecmascript/mem/heap-inl.h"
#include "ecmascript/mem/mem.h"
#include "ecmascript/mem/space-inl.h"
#include "ecmascript/mem/tlab_allocator-inl.h"
#include "ecmascript/mem/visitor.h"
#include "ecmascript/mem/gc_stats.h"
#include "ecmascript/ecma_string_table.h"
#include "ecmascript/runtime_call_id.h"

namespace panda::ecmascript {
void ParallelEvacuator::Initialize()
{
    MEM_ALLOCATE_AND_GC_TRACE(heap_->GetEcmaVM(), ParallelEvacuatorInitialize);
    waterLine_ = heap_->GetNewSpace()->GetWaterLine();
    heap_->SwapNewSpace();
    allocator_ = new TlabAllocator(heap_);
    promotedSize_ = 0;
}

void ParallelEvacuator::Finalize()
{
    MEM_ALLOCATE_AND_GC_TRACE(heap_->GetEcmaVM(), ParallelEvacuatorFinalize);
    delete allocator_;
}

void ParallelEvacuator::Evacuate()
{
    Initialize();
    EvacuateSpace();
    UpdateReference();
    Finalize();
}

void ParallelEvacuator::UpdateTrackInfo()
{
    for (uint32_t i = 0; i <= MAX_TASKPOOL_THREAD_NUM; i++) {
        auto &trackInfoSet = ArrayTrackInfoSet(i);
        for (auto &each : trackInfoSet) {
            auto trackInfoVal = JSTaggedValue(each);
            if (!trackInfoVal.IsHeapObject() || !trackInfoVal.IsWeak()) {
                continue;
            }
            auto trackInfo = trackInfoVal.GetWeakReferentUnChecked();
            trackInfo = UpdateAddressAfterEvacation(trackInfo);
            if (trackInfo) {
                heap_->GetEcmaVM()->GetPGOProfiler()->UpdateTrackSpaceFlag(trackInfo, RegionSpaceFlag::IN_OLD_SPACE);
            }
        }
        trackInfoSet.clear();
    }
}

void ParallelEvacuator::EvacuateSpace()
{
    TRACE_GC(GCStats::Scope::ScopeId::EvacuateSpace, heap_->GetEcmaVM()->GetEcmaGCStats());
    ECMA_BYTRACE_NAME(HITRACE_TAG_ARK, "GC::EvacuateSpace");
    MEM_ALLOCATE_AND_GC_TRACE(heap_->GetEcmaVM(), ParallelEvacuator);
    heap_->GetFromSpaceDuringEvacuation()->EnumerateRegions([this] (Region *current) {
        AddWorkload(std::make_unique<EvacuateWorkload>(this, current));
    });
    heap_->GetOldSpace()->EnumerateCollectRegionSet(
        [this](Region *current) {
            AddWorkload(std::make_unique<EvacuateWorkload>(this, current));
        });
    if (heap_->IsParallelGCEnabled()) {
        LockHolder holder(mutex_);
        parallel_ = CalculateEvacuationThreadNum();
        for (int i = 0; i < parallel_; i++) {
            Taskpool::GetCurrentTaskpool()->PostTask(
                std::make_unique<EvacuationTask>(heap_->GetJSThread()->GetThreadId(), this));
        }
    }

    EvacuateSpace(allocator_, 0, true);
    WaitFinished();
    if (heap_->GetJSThread()->IsPGOProfilerEnable()) {
        UpdateTrackInfo();
    }
}

bool ParallelEvacuator::EvacuateSpace(TlabAllocator *allocator, uint32_t threadIndex, bool isMain)
{
    std::unique_ptr<Workload> region = GetWorkloadSafe();
    auto &arrayTrackInfoSet = ArrayTrackInfoSet(threadIndex);
    while (region != nullptr) {
        EvacuateRegion(allocator, region->GetRegion(), arrayTrackInfoSet);
        region = GetWorkloadSafe();
    }
    allocator->Finalize();
    if (!isMain) {
        LockHolder holder(mutex_);
        if (--parallel_ <= 0) {
            condition_.SignalAll();
        }
    }
    return true;
}

void ParallelEvacuator::EvacuateRegion(TlabAllocator *allocator, Region *region,
                                       std::unordered_set<JSTaggedType> &trackSet)
{
    bool isInOldGen = region->InOldSpace();
    bool isBelowAgeMark = region->BelowAgeMark();
    bool pgoEnabled = heap_->GetJSThread()->IsPGOProfilerEnable();
    size_t promotedSize = 0;
    if (!isBelowAgeMark && !isInOldGen && IsWholeRegionEvacuate(region)) {
        if (heap_->MoveYoungRegionSync(region)) {
            return;
        }
    }
    region->IterateAllMarkedBits([this, &region, &isInOldGen, &isBelowAgeMark, &pgoEnabled,
                                  &promotedSize, &allocator, &trackSet](void *mem) {
        ASSERT(region->InRange(ToUintPtr(mem)));
        auto header = reinterpret_cast<TaggedObject *>(mem);
        auto klass = header->GetClass();
        auto size = klass->SizeFromJSHClass(header);

        uintptr_t address = 0;
        bool actualPromoted = false;
        bool hasAgeMark = isBelowAgeMark || (region->HasAgeMark() && ToUintPtr(mem) < waterLine_);
        if (hasAgeMark) {
            address = allocator->Allocate(size, OLD_SPACE);
            actualPromoted = true;
            promotedSize += size;
        } else if (isInOldGen) {
            address = allocator->Allocate(size, OLD_SPACE);
            actualPromoted = true;
        } else {
            address = allocator->Allocate(size, SEMI_SPACE);
            if (address == 0) {
                address = allocator->Allocate(size, OLD_SPACE);
                actualPromoted = true;
                promotedSize += size;
            }
        }
        LOG_ECMA_IF(address == 0, FATAL) << "Evacuate object failed:" << size;

        if (memcpy_s(ToVoidPtr(address), size, ToVoidPtr(ToUintPtr(mem)), size) != EOK) {
            LOG_FULL(FATAL) << "memcpy_s failed";
        }
        heap_->OnMoveEvent(reinterpret_cast<uintptr_t>(mem), reinterpret_cast<TaggedObject *>(address), size);
        if (pgoEnabled) {
            if (actualPromoted && klass->IsJSArray()) {
                auto trackInfo = JSArray::Cast(header)->GetTrackInfo();
                trackSet.emplace(trackInfo.GetRawData());
            }
        }
        Barriers::SetPrimitive(header, 0, MarkWord::FromForwardingAddress(address));

        if (UNLIKELY(heap_->ShouldVerifyHeap())) {
            VerifyHeapObject(reinterpret_cast<TaggedObject *>(address));
        }
        if (actualPromoted) {
            SetObjectFieldRSet(reinterpret_cast<TaggedObject *>(address), klass);
        }
        if (region->HasLocalToShareRememberedSet()) {
            UpdateLocalToShareRSet(reinterpret_cast<TaggedObject *>(address), klass);
        }
    });
    promotedSize_.fetch_add(promotedSize);
}

void ParallelEvacuator::VerifyHeapObject(TaggedObject *object)
{
    auto klass = object->GetClass();
    ObjectXRay::VisitObjectBody<VisitType::OLD_GC_VISIT>(object, klass,
        [&](TaggedObject *root, ObjectSlot start, ObjectSlot end, VisitObjectArea area) {
            if (area == VisitObjectArea::IN_OBJECT) {
                if (VisitBodyInObj(root, start, end, [&](ObjectSlot slot) { VerifyValue(object, slot); })) {
                    return;
                };
            }
            for (ObjectSlot slot = start; slot < end; slot++) {
                VerifyValue(object, slot);
            }
        });
}

void ParallelEvacuator::VerifyValue(TaggedObject *object, ObjectSlot slot)
{
    JSTaggedValue value(slot.GetTaggedType());
    if (value.IsHeapObject()) {
        if (value.IsWeakForHeapObject()) {
            return;
        }
        Region *objectRegion = Region::ObjectAddressToRange(value.GetTaggedObject());
        if (objectRegion->InSharedHeap()) {
            return;
        }
        if (!heap_->IsConcurrentFullMark() && !objectRegion->InYoungSpace()) {
            return;
        }
        if (!objectRegion->Test(value.GetTaggedObject()) && !objectRegion->InAppSpawnSpace()) {
            LOG_GC(FATAL) << "Miss mark value: " << value.GetTaggedObject()
                                << ", body address:" << slot.SlotAddress()
                                << ", header address:" << object;
        }
    }
}

void ParallelEvacuator::UpdateReference()
{
    TRACE_GC(GCStats::Scope::ScopeId::UpdateReference, heap_->GetEcmaVM()->GetEcmaGCStats());
    MEM_ALLOCATE_AND_GC_TRACE(heap_->GetEcmaVM(), ParallelUpdateReference);
    // Update reference pointers
    uint32_t youngeRegionMoveCount = 0;
    uint32_t youngeRegionCopyCount = 0;
    uint32_t oldRegionCount = 0;
    heap_->GetNewSpace()->EnumerateRegions([&] (Region *current) {
        if (current->InNewToNewSet()) {
            AddWorkload(std::make_unique<UpdateAndSweepNewRegionWorkload>(this, current));
            youngeRegionMoveCount++;
        } else {
            AddWorkload(std::make_unique<UpdateNewRegionWorkload>(this, current));
            youngeRegionCopyCount++;
        }
    });
    heap_->EnumerateOldSpaceRegions([this, &oldRegionCount] (Region *current) {
        if (current->InCollectSet()) {
            return;
        }
        AddWorkload(std::make_unique<UpdateRSetWorkload>(this, current));
        oldRegionCount++;
    });
    heap_->EnumerateSnapshotSpaceRegions([this] (Region *current) {
        AddWorkload(std::make_unique<UpdateRSetWorkload>(this, current));
    });
    LOG_GC(DEBUG) << "UpdatePointers statistic: younge space region compact moving count:"
                        << youngeRegionMoveCount
                        << "younge space region compact coping count:" << youngeRegionCopyCount
                        << "old space region count:" << oldRegionCount;

    if (heap_->IsParallelGCEnabled()) {
        LockHolder holder(mutex_);
        parallel_ = CalculateUpdateThreadNum();
        for (int i = 0; i < parallel_; i++) {
            Taskpool::GetCurrentTaskpool()->PostTask(
                std::make_unique<UpdateReferenceTask>(heap_->GetJSThread()->GetThreadId(), this));
        }
    }

    UpdateRoot();
    UpdateWeakReference();
    ProcessWorkloads(true);
    WaitFinished();
}

void ParallelEvacuator::UpdateRoot()
{
    MEM_ALLOCATE_AND_GC_TRACE(heap_->GetEcmaVM(), UpdateRoot);
    RootVisitor gcUpdateYoung = [this]([[maybe_unused]] Root type, ObjectSlot slot) {
        UpdateObjectSlot(slot);
    };
    RootRangeVisitor gcUpdateRangeYoung = [this]([[maybe_unused]] Root type, ObjectSlot start, ObjectSlot end) {
        for (ObjectSlot slot = start; slot < end; slot++) {
            UpdateObjectSlot(slot);
        }
    };
    RootBaseAndDerivedVisitor gcUpdateDerived =
        []([[maybe_unused]] Root type, ObjectSlot base, ObjectSlot derived, uintptr_t baseOldObject) {
        if (JSTaggedValue(base.GetTaggedType()).IsHeapObject()) {
            derived.Update(base.GetTaggedType() + derived.GetTaggedType() - baseOldObject);
        }
    };

    ObjectXRay::VisitVMRoots(heap_->GetEcmaVM(), gcUpdateYoung, gcUpdateRangeYoung, gcUpdateDerived);
}

void ParallelEvacuator::UpdateRecordWeakReference()
{
    auto totalThreadCount = Taskpool::GetCurrentTaskpool()->GetTotalThreadNum() + 1;
    for (uint32_t i = 0; i < totalThreadCount; i++) {
        ProcessQueue *queue = heap_->GetWorkManager()->GetWeakReferenceQueue(i);

        while (true) {
            auto obj = queue->PopBack();
            if (UNLIKELY(obj == nullptr)) {
                break;
            }
            ObjectSlot slot(ToUintPtr(obj));
            JSTaggedValue value(slot.GetTaggedType());
            if (value.IsWeak()) {
                UpdateWeakObjectSlot(value.GetTaggedWeakRef(), slot);
            }
        }
    }
}

void ParallelEvacuator::UpdateWeakReference()
{
    MEM_ALLOCATE_AND_GC_TRACE(heap_->GetEcmaVM(), UpdateWeakReference);
    ECMA_BYTRACE_NAME(HITRACE_TAG_ARK, "GC::UpdateWeakReference");
    UpdateRecordWeakReference();
    bool isFullMark = heap_->IsConcurrentFullMark();
    WeakRootVisitor gcUpdateWeak = [isFullMark](TaggedObject *header) {
        Region *objectRegion = Region::ObjectAddressToRange(reinterpret_cast<TaggedObject *>(header));
        if (!objectRegion) {
            LOG_GC(ERROR) << "PartialGC updateWeakReference: region is nullptr, header is " << header;
            return reinterpret_cast<TaggedObject *>(ToUintPtr(nullptr));
        }
        // The weak object in shared heap is always alive during partialGC.
        if (objectRegion->InSharedHeap()) {
            return header;
        }
        if (objectRegion->InYoungSpaceOrCSet()) {
            if (objectRegion->InNewToNewSet()) {
                if (objectRegion->Test(header)) {
                    return header;
                }
            } else {
                MarkWord markWord(header);
                if (markWord.IsForwardingAddress()) {
                    return markWord.ToForwardingAddress();
                }
            }
            return reinterpret_cast<TaggedObject *>(ToUintPtr(nullptr));
        }
        if (isFullMark) {
            if (objectRegion->GetMarkGCBitset() == nullptr || !objectRegion->Test(header)) {
                return reinterpret_cast<TaggedObject *>(ToUintPtr(nullptr));
            }
        }
        return header;
    };

    heap_->GetEcmaVM()->GetJSThread()->IterateWeakEcmaGlobalStorage(gcUpdateWeak);
    heap_->GetEcmaVM()->ProcessReferences(gcUpdateWeak);
}

void ParallelEvacuator::UpdateRSet(Region *region)
{
    auto cb = [this](void *mem) -> bool {
        ObjectSlot slot(ToUintPtr(mem));
        return UpdateOldToNewObjectSlot(slot);
    };
    if (heap_->GetSweeper()->IsSweeping()) {
        if (region->IsGCFlagSet(RegionGCFlags::HAS_BEEN_SWEPT)) {
            // Region is safe while update remember set
            region->MergeRSetForConcurrentSweeping();
        } else {
            region->AtomicIterateAllSweepingRSetBits(cb);
        }
    }
    region->IterateAllOldToNewBits(cb);
    region->IterateAllCrossRegionBits([this](void *mem) {
        ObjectSlot slot(ToUintPtr(mem));
        UpdateObjectSlot(slot);
    });
    region->ClearCrossRegionRSet();
}

void ParallelEvacuator::UpdateNewRegionReference(Region *region)
{
    Region *current = heap_->GetNewSpace()->GetCurrentRegion();
    auto curPtr = region->GetBegin();
    uintptr_t endPtr = 0;
    if (region == current) {
        auto top = heap_->GetNewSpace()->GetTop();
        endPtr = curPtr + region->GetAllocatedBytes(top);
    } else {
        endPtr = curPtr + region->GetAllocatedBytes();
    }

    size_t objSize = 0;
    while (curPtr < endPtr) {
        auto freeObject = FreeObject::Cast(curPtr);
        // If curPtr is freeObject, It must to mark unpoison first.
        ASAN_UNPOISON_MEMORY_REGION(reinterpret_cast<void *>(freeObject), TaggedObject::TaggedObjectSize());
        if (!freeObject->IsFreeObject()) {
            auto obj = reinterpret_cast<TaggedObject *>(curPtr);
            auto klass = obj->GetClass();
            UpdateNewObjectField(obj, klass);
            objSize = klass->SizeFromJSHClass(obj);
        } else {
            freeObject->AsanUnPoisonFreeObject();
            objSize = freeObject->Available();
            freeObject->AsanPoisonFreeObject();
        }
        curPtr += objSize;
        CHECK_OBJECT_SIZE(objSize);
    }
    CHECK_REGION_END(curPtr, endPtr);
}

void ParallelEvacuator::UpdateAndSweepNewRegionReference(Region *region)
{
    uintptr_t freeStart = region->GetBegin();
    uintptr_t freeEnd = freeStart + region->GetAllocatedBytes();
    region->IterateAllMarkedBits([&](void *mem) {
        ASSERT(region->InRange(ToUintPtr(mem)));
        auto header = reinterpret_cast<TaggedObject *>(mem);
        JSHClass *klass = header->GetClass();
        UpdateNewObjectField(header, klass);

        uintptr_t freeEnd = ToUintPtr(mem);
        if (freeStart != freeEnd) {
            size_t freeSize = freeEnd - freeStart;
            FreeObject::FillFreeObject(heap_, freeStart, freeSize);
            SemiSpace *toSpace = heap_->GetNewSpace();
            toSpace->DecreaseSurvivalObjectSize(freeSize);
        }

        freeStart = freeEnd + klass->SizeFromJSHClass(header);
    });
    CHECK_REGION_END(freeStart, freeEnd);
    if (freeStart < freeEnd) {
        FreeObject::FillFreeObject(heap_, freeStart, freeEnd - freeStart);
    }
}

void ParallelEvacuator::UpdateNewObjectField(TaggedObject *object, JSHClass *cls)
{
    ObjectXRay::VisitObjectBody<VisitType::OLD_GC_VISIT>(object, cls,
        [this](TaggedObject *root, ObjectSlot start, ObjectSlot end, VisitObjectArea area) {
            if (area == VisitObjectArea::IN_OBJECT) {
                if (VisitBodyInObj(root, start, end, [&](ObjectSlot slot) { UpdateObjectSlot(slot); })) {
                    return;
                };
            }
            for (ObjectSlot slot = start; slot < end; slot++) {
                UpdateObjectSlot(slot);
            }
        });
}

void ParallelEvacuator::WaitFinished()
{
    MEM_ALLOCATE_AND_GC_TRACE(heap_->GetEcmaVM(), WaitUpdateFinished);
    if (parallel_ > 0) {
        LockHolder holder(mutex_);
        while (parallel_ > 0) {
            condition_.Wait(&mutex_);
        }
    }
}

bool ParallelEvacuator::ProcessWorkloads(bool isMain)
{
    std::unique_ptr<Workload> region = GetWorkloadSafe();
    while (region != nullptr) {
        region->Process(isMain);
        region = GetWorkloadSafe();
    }
    if (!isMain) {
        LockHolder holder(mutex_);
        if (--parallel_ <= 0) {
            condition_.SignalAll();
        }
    }
    return true;
}

ParallelEvacuator::EvacuationTask::EvacuationTask(int32_t id, ParallelEvacuator *evacuator)
    : Task(id), evacuator_(evacuator)
{
    allocator_ = new TlabAllocator(evacuator->heap_);
}

ParallelEvacuator::EvacuationTask::~EvacuationTask()
{
    delete allocator_;
}

bool ParallelEvacuator::EvacuationTask::Run(uint32_t threadIndex)
{
    return evacuator_->EvacuateSpace(allocator_, threadIndex);
}

bool ParallelEvacuator::UpdateReferenceTask::Run([[maybe_unused]] uint32_t threadIndex)
{
    evacuator_->ProcessWorkloads(false);
    return true;
}

bool ParallelEvacuator::EvacuateWorkload::Process([[maybe_unused]] bool isMain)
{
    return true;
}

bool ParallelEvacuator::UpdateRSetWorkload::Process([[maybe_unused]] bool isMain)
{
    GetEvacuator()->UpdateRSet(GetRegion());
    return true;
}

bool ParallelEvacuator::UpdateNewRegionWorkload::Process([[maybe_unused]] bool isMain)
{
    GetEvacuator()->UpdateNewRegionReference(GetRegion());
    return true;
}

bool ParallelEvacuator::UpdateAndSweepNewRegionWorkload::Process([[maybe_unused]] bool isMain)
{
    GetEvacuator()->UpdateAndSweepNewRegionReference(GetRegion());
    return true;
}
}  // namespace panda::ecmascript
