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

#ifndef ECMASCRIPT_JSPANDAFILE_JS_PANDAFILE_H
#define ECMASCRIPT_JSPANDAFILE_JS_PANDAFILE_H

#include "ecmascript/common.h"
#include "ecmascript/js_function.h"
#include "ecmascript/js_method.h"
#include "ecmascript/jspandafile/constpool_value.h"
#include "ecmascript/mem/c_containers.h"
#include "libpandafile/file.h"

namespace panda {
namespace ecmascript {
class JSPandaFile {
public:
    static constexpr char ENTRY_FUNCTION_NAME[] = "func_main_0";
    static constexpr char ENTRY_MAIN_FUNCTION[] = "_GLOBAL::func_main_0";
    static constexpr char MODULE_CLASS[] = "L_ESModuleRecord;";
    static constexpr char TS_TYPES_CLASS[] = "L_ESTypeInfoRecord;";
    static constexpr char COMMONJS_CLASS[] = "L_CommonJsRecord;";
    static constexpr char TYPE_FLAG[] = "typeFlag";
    static constexpr char TYPE_SUMMARY_INDEX[] = "typeSummaryIndex";

    JSPandaFile(const panda_file::File *pf, const CString &descriptor);
    ~JSPandaFile();

    const CString &GetJSPandaFileDesc() const
    {
        return desc_;
    }

    const panda_file::File *GetPandaFile() const
    {
        return pf_;
    }

    JSMethod *GetMethods() const
    {
        return methods_;
    }

    void SetMethodToMap(JSMethod *method)
    {
        if (method != nullptr) {
            methodMap_.emplace(method->GetMethodId().GetOffset(), method);
        }
    }

    uint32_t GetNumMethods() const
    {
        return numMethods_;
    }

    uint32_t GetConstpoolIndex() const
    {
        return constpoolIndex_;
    }

    uint32_t GetMainMethodIndex() const
    {
        return mainMethodIndex_;
    }

    const CUnorderedMap<uint32_t, uint64_t> &GetConstpoolMap() const
    {
        return constpoolMap_;
    }

    uint32_t GetOrInsertConstantPool(ConstPoolType type, uint32_t offset);

    void UpdateMainMethodIndex(uint32_t mainMethodIndex)
    {
        mainMethodIndex_ = mainMethodIndex;
    }

    JSMethod *FindMethods(uint32_t offset) const;

    Span<const uint32_t> GetClasses() const
    {
        return pf_->GetClasses();
    }

    bool PUBLIC_API IsModule() const;

    bool IsCjs() const;

    bool HasTSTypes() const
    {
        return hasTSTypes_;
    }

    void SetLoadedAOTStatus(bool status)
    {
        isLoadedAOT_ = status;
    }

    bool IsLoadedAOT() const
    {
        return isLoadedAOT_;
    }

    uint32_t GetTypeSummaryIndex() const
    {
        return typeSummaryIndex_;
    }

    uint32_t GetFileUniqId() const
    {
        return static_cast<uint32_t>(GetPandaFile()->GetUniqId());
    }

    inline void SetUniqueMethodMap(const JSMethod *method, const uint8_t *insans)
    {
        if (method != nullptr) {
            auto offset = method->GetMethodId().GetOffset();
            if (pcToUniqMethodOffset_.find(insans) == pcToUniqMethodOffset_.end()) {
                pcToUniqMethodOffset_.emplace(insans, offset);
            }
            dupMethodOffsetToUniqMethodOffset_.emplace(offset, pcToUniqMethodOffset_.at(insans));
        }
    }

    inline uint32_t GetUniqueMethod(const JSHandle<JSFunction> &func) const
    {
        auto method = func->GetMethod();
        ASSERT(method != nullptr);
        auto methodOffset = method->GetMethodId().GetOffset();
        return dupMethodOffsetToUniqMethodOffset_.at(methodOffset);
    }

private:
    void Initialize();
    uint32_t constpoolIndex_ {0};
    CUnorderedMap<uint32_t, uint64_t> constpoolMap_;
    uint32_t numMethods_ {0};
    uint32_t mainMethodIndex_ {0};
    JSMethod *methods_ {nullptr};
    CUnorderedMap<uint32_t, JSMethod *> methodMap_;
    CUnorderedMap<uint32_t, uint32_t> dupMethodOffsetToUniqMethodOffset_;
    CUnorderedMap<const uint8_t *, uint32_t> pcToUniqMethodOffset_;
    const panda_file::File *pf_ {nullptr};
    CString desc_;
    bool isModule_ {false};
    bool isCjs_ {false};
    bool hasTSTypes_ {false};
    bool isLoadedAOT_ {false};
    uint32_t typeSummaryIndex_ {0};
};
}  // namespace ecmascript
}  // namespace panda
#endif // ECMASCRIPT_JSPANDAFILE_JS_PANDAFILE_H
