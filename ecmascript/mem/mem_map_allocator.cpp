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

#include "ecmascript/mem/mem_map_allocator.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/platform/map.h"
#include "ecmascript/platform/os.h"

namespace panda::ecmascript {
MemMapAllocator *MemMapAllocator::GetInstance()
{
    static MemMapAllocator *vmAllocator_ = new MemMapAllocator();
    return vmAllocator_;
}

MemMap MemMapAllocator::Allocate(const uint32_t threadId, size_t size, size_t alignment,
                                 const std::string &spaceName, bool regular, bool isMachineCode)
{
    MemMap mem;
    if (regular) {
        mem = memMapPool_.GetRegularMemFromCommitted(size);
        if (mem.GetMem() != nullptr) {
            int prot = isMachineCode ? PAGE_PROT_EXEC_READWRITE : PAGE_PROT_READWRITE;
            PageTagType type = isMachineCode ? PageTagType::MACHINE_CODE : PageTagType::HEAP;
            if (!PageProtect(mem.GetMem(), mem.GetSize(), prot)) {
                return MemMap();
            }
            PageTag(mem.GetMem(), size, type, spaceName, threadId);
            return mem;
        }

        if (UNLIKELY(memMapTotalSize_ + size > capacity_)) {
            LOG_GC(ERROR) << "memory map overflow";
            return MemMap();
        }
        mem = memMapPool_.GetMemFromCache(size);
        if (mem.GetMem() != nullptr) {
            memMapTotalSize_ += size;
            int prot = isMachineCode ? PAGE_PROT_EXEC_READWRITE : PAGE_PROT_READWRITE;
            PageTagType type = isMachineCode ? PageTagType::MACHINE_CODE : PageTagType::HEAP;
            if (!PageProtect(mem.GetMem(), mem.GetSize(), prot)) {
                return MemMap();
            }
            PageTag(mem.GetMem(), size, type, spaceName, threadId);
            return mem;
        }
        mem = PageMap(REGULAR_REGION_MMAP_SIZE, PAGE_PROT_NONE, alignment);
        memMapPool_.InsertMemMap(mem);
        mem = memMapPool_.SplitMemFromCache(mem);
    } else {
        if (UNLIKELY(memMapTotalSize_ + size > capacity_)) {
            LOG_GC(ERROR) << "memory map overflow";
            return MemMap();
        }
        mem = memMapFreeList_.GetMemFromList(size);
    }
    if (mem.GetMem() != nullptr) {
        int prot = isMachineCode ? PAGE_PROT_EXEC_READWRITE : PAGE_PROT_READWRITE;
        PageTagType type = isMachineCode ? PageTagType::MACHINE_CODE : PageTagType::HEAP;
        if (!PageProtect(mem.GetMem(), mem.GetSize(), prot)) {
            return MemMap();
        }
        PageTag(mem.GetMem(), mem.GetSize(), type, spaceName, threadId);
        memMapTotalSize_ += mem.GetSize();
    }
    return mem;
}

void MemMapAllocator::CacheOrFree(void *mem, size_t size, bool isRegular, size_t cachedSize)
{
    if (isRegular && !memMapPool_.IsRegularCommittedFull(cachedSize)) {
        // Cache regions to accelerate allocation.
        // Clear ThreadId tag and tag the mem with ARKTS HEAP.
        PageClearTag(mem, size);
        PageTag(mem, size, PageTagType::HEAP);
        memMapPool_.AddMemToCommittedCache(mem, size);
        return;
    }
    Free(mem, size, isRegular);
    if (isRegular && memMapPool_.ShouldFreeMore(cachedSize) > 0) {
        int freeNum = memMapPool_.ShouldFreeMore(cachedSize);
        for (int i = 0; i < freeNum; i++) {
            void *freeMem = memMapPool_.GetRegularMemFromCommitted(size).GetMem();
            if (freeMem != nullptr) {
                Free(freeMem, size, isRegular);
            } else {
                return;
            }
        }
    }
}

void MemMapAllocator::Free(void *mem, size_t size, bool isRegular)
{
    memMapTotalSize_ -= size;
    PageTag(mem, size, PageTagType::MEMPOOL_CACHE);
    if (!PageProtect(mem, size, PAGE_PROT_NONE)) {
        return;
    }
    PageRelease(mem, size);
    if (isRegular) {
        memMapPool_.AddMemToCache(mem, size);
    } else {
        memMapFreeList_.AddMemToList(MemMap(mem, size));
    }
}

void MemMapAllocator::AdapterSuitablePoolCapacity()
{
    size_t physicalSize = PhysicalSize();
    capacity_ = std::min<size_t>(physicalSize * DEFAULT_CAPACITY_RATE, MAX_MEM_POOL_CAPACITY);
    LOG_GC(INFO) << "Ark Auto adapter memory pool capacity:" << capacity_;
}
}  // namespace panda::ecmascript
