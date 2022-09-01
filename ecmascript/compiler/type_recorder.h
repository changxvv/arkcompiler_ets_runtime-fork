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

#ifndef ECMASCRIPT_COMPILER_TYPE_RECORDER_H
#define ECMASCRIPT_COMPILER_TYPE_RECORDER_H

#include "ecmascript/compiler/type.h"
#include "ecmascript/jspandafile/js_pandafile.h"
#include "ecmascript/ts_types/ts_manager.h"
#include "libpandafile/file-inl.h"
#include "ecmascript/method.h"
#include "libpandafile/method_data_accessor-inl.h"

namespace panda::ecmascript::kungfu {
class TypeRecorder {
public:
    explicit TypeRecorder(const JSPandaFile *jsPandaFile, const MethodLiteral *methodLiteral, TSManager *tsManager);
    ~TypeRecorder() = default;

    GateType GetType(const int32_t offset) const;
    GateType GetArgType(const int32_t argIndex) const;
    GateType UpdateType(const int32_t offset, const GateType &type) const;

private:
    void LoadTypes(const JSPandaFile *jsPandaFile, const MethodLiteral *methodLiteral, TSManager *tsManager);
    
    inline int32_t GetArgOffset(const int32_t argIndex) const
    {
        return -argIndex - 1;
    }

    std::unordered_map<int32_t, GateType> bcOffsetGtMap_ {};
};
}  // panda::ecmascript::kungfu
#endif  // ECMASCRIPT_COMPILER_TYPE_RECORDER_H
