/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef ECMASCRIPT_COMPILER_AOT_FILE_MODULE_SECTION_DES_H
#define ECMASCRIPT_COMPILER_AOT_FILE_MODULE_SECTION_DES_H

#include <cstdint>
#include <memory>

#include "ecmascript/compiler/aot_file/binary_buffer_parser.h"
#include "ecmascript/compiler/binary_section.h"

namespace panda::ecmascript {
class ModuleSectionDes {
public:
    struct ModuleRegionInfo {
        uint32_t startIndex {0};
        uint32_t funcCount {0};
        uint32_t rodataSize {0};
        uint32_t textSize {0};
        uint32_t stackMapSize {0};
    };
    static std::string GetSecName(ElfSecName idx);

    void SetArkStackMapPtr(std::shared_ptr<uint8_t> ptr)
    {
        arkStackMapPtr_ = std::move(ptr);
    }

    std::shared_ptr<uint8_t> GetArkStackMapSharePtr() const
    {
        return arkStackMapPtr_;
    }

    std::map<ElfSecName, std::pair<uint64_t, uint32_t>> &GetSectionsInfo()
    {
        return sectionsInfo_;
    }

    const std::map<ElfSecName, std::pair<uint64_t, uint32_t>> &GetSectionsInfo() const
    {
        return sectionsInfo_;
    }

    void SetArkStackMapPtr(uint8_t *ptr)
    {
        arkStackMapRawPtr_ = ptr;
    }

    uint8_t *GetArkStackMapRawPtr() const
    {
        return arkStackMapRawPtr_;
    }

    void SetArkStackMapSize(uint32_t size)
    {
        arkStackMapSize_ = size;
    }

    uint32_t GetArkStackMapSize() const
    {
        return arkStackMapSize_;
    }

    void SetStartIndex(uint32_t index)
    {
        startIndex_ = index;
    }

    uint32_t GetStartIndex() const
    {
        return startIndex_;
    }

    void SetFuncCount(uint32_t cnt)
    {
        funcCount_ = cnt;
    }

    uint32_t GetFuncCount() const
    {
        return funcCount_;
    }

    ModuleSectionDes() = default;

    void EraseSec(ElfSecName idx)
    {
        sectionsInfo_.erase(idx);
    }

    void SetSecAddrAndSize(ElfSecName idx, uint64_t addr, uint32_t size)
    {
        sectionsInfo_[idx].first = addr;
        sectionsInfo_[idx].second = size;
    }

    uint64_t GetSecAddr(const ElfSecName idx) const
    {
        auto it = sectionsInfo_.find(idx);
        return it == sectionsInfo_.end() ? 0 : it->second.first;
    }

    uint32_t GetSecSize(const ElfSecName idx) const
    {
        auto it = sectionsInfo_.find(idx);
        return it == sectionsInfo_.end() ? 0 : it->second.second;
    }

    uint32_t GetSecInfosSize() const
    {
        return sectionsInfo_.size();
    }

    bool ContainCode(uintptr_t pc) const
    {
        uint64_t stubStartAddr = GetSecAddr(ElfSecName::TEXT);
        uint64_t stubEndAddr = stubStartAddr + GetSecSize(ElfSecName::TEXT);
        return (pc >= stubStartAddr && pc <= stubEndAddr);
    }

private:
    static constexpr int DECIMAL_LENS = 2;
    static constexpr int HUNDRED_TIME = 100;
    static constexpr int PERCENT_LENS = 4;

    std::map<ElfSecName, std::pair<uint64_t, uint32_t>> sectionsInfo_ {};
    uint32_t startIndex_ {static_cast<uint32_t>(-1)};  // record current module first function index in AOTFileInfo
    uint32_t funcCount_ {0};
    std::shared_ptr<uint8_t> arkStackMapPtr_ {nullptr};
    uint32_t arkStackMapSize_ {0};
    uint8_t *arkStackMapRawPtr_ {nullptr};
};
}  // namespace panda::ecmascript
#endif  // ECMASCRIPT_COMPILER_AOT_FILE_MODULE_SECTION_DES_H
