/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef ECMASCRIPT_MEM_BARRIERS_INL_H
#define ECMASCRIPT_MEM_BARRIERS_INL_H

#include "ecmascript/mem/barriers.h"
#include "ecmascript/mem/mem.h"
#include "ecmascript/mem/region-inl.h"
#include "ecmascript/runtime_api.h"

namespace panda::ecmascript {
static inline void MarkingBarrier(uintptr_t slotAddr, Region *objectRegion, TaggedObject *value,
    Region *valueRegion)
{
    auto heap = valueRegion->GetHeap();
    bool isOnlySemi = heap->IsSemiMarkNeeded();
    if (!JSTaggedValue(value).IsWeak()) {
        if (isOnlySemi && !valueRegion->InYoungGeneration()) {
            return;
        }
        auto valueBitmap = valueRegion->GetOrCreateMarkBitmap();
        if (!RuntimeApi::AtomicTestAndSet(valueBitmap, value)) {
            RuntimeApi::PushWorkList(heap->GetWorkList(), 0, value, valueRegion);
        }
    }
    if (!isOnlySemi && !objectRegion->InYoungAndCSetGeneration() && valueRegion->InCollectSet()) {
        auto set = objectRegion->GetOrCreateCrossRegionRememberedSet();
        RuntimeApi::AtomicInsertCrossRegionRememberedSet(set, slotAddr);
    }
}

static inline void WriteBarrier(void *obj, size_t offset, JSTaggedType value)
{
    ASSERT(value != JSTaggedValue::VALUE_UNDEFINED);
    Region *objectRegion = Region::ObjectAddressToRange(static_cast<TaggedObject *>(obj));
    Region *valueRegion = Region::ObjectAddressToRange(reinterpret_cast<TaggedObject *>(value));
    uintptr_t slotAddr = ToUintPtr(obj) + offset;
    if (!objectRegion->InYoungGeneration() && valueRegion->InYoungGeneration()) {
        // Should align with '8' in 64 and 32 bit platform
        ASSERT((slotAddr % static_cast<uint8_t>(MemAlignment::MEM_ALIGN_OBJECT)) == 0);
        objectRegion->InsertOldToNewRememberedSet(slotAddr);
    }

    if (valueRegion->IsMarking()) {
        MarkingBarrier(slotAddr, objectRegion, reinterpret_cast<TaggedObject *>(value), valueRegion);
    }
}

/* static */
// CODECHECK-NOLINTNEXTLINE(C_RULE_ID_COMMENT_LOCATION)
// default value for need_write_barrier is true
template<bool need_write_barrier>
inline void Barriers::SetDynObject([[maybe_unused]] const JSThread *thread, void *obj, size_t offset,
                                   JSTaggedType value)
{
    // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
    *reinterpret_cast<JSTaggedType *>(reinterpret_cast<uintptr_t>(obj) + offset) = value;
    WriteBarrier(obj, offset, value);
}
}  // namespace panda::ecmascript

#endif  // ECMASCRIPT_MEM_BARRIERS_INL_H
