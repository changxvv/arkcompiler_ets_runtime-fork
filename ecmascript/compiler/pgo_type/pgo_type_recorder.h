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

#ifndef ECMASCRIPT_COMPILER_PGO_TYPE_PGO_TYPE_RECORD_H
#define ECMASCRIPT_COMPILER_PGO_TYPE_PGO_TYPE_RECORD_H

#include "ecmascript/pgo_profiler/pgo_profiler_decoder.h"
#include "ecmascript/pgo_profiler/types/pgo_profiler_type.h"

namespace panda::ecmascript::kungfu {
using PGOProfilerDecoder = pgo::PGOProfilerDecoder;
using PGOHClassTreeDesc = pgo::PGOHClassTreeDesc;
using PGODefineOpType = pgo::PGODefineOpType;
using RootHClassLayoutDesc = pgo::RootHClassLayoutDesc;
using PGORWOpType = pgo::PGORWOpType;

class PGOTypeRecorder {
public:
    PGOTypeRecorder(const PGOProfilerDecoder &decoder, const JSPandaFile *jsPandaFile, uint32_t methodOffset);
    ~PGOTypeRecorder() = default;

    std::vector<ElementsKind> GetElementsKindsForUser(int32_t offset) const;
    ElementsKind GetElementsKindForCreater(int32_t offset) const;

    PGOTypeRef GetPGOType(int32_t offset) const;

    inline PGODefineOpType GetPGODefOpType(int32_t bcOffset) const
    {
        auto typeIter = bcOffsetPGODefOpTypeMap_.find(bcOffset);
        if (typeIter == bcOffsetPGODefOpTypeMap_.end()) {
            return PGODefineOpType(ProfileType::PROFILE_TYPE_NONE);
        }
        return *(typeIter->second);
    }

    inline bool GetHClassTreeDesc(PGOSampleType type, PGOHClassTreeDesc **desc) const
    {
        return decoder_.GetHClassTreeDesc(type, desc);
    }

private:
    const PGOProfilerDecoder &decoder_;
    std::unordered_map<int32_t, const PGOSampleType*> bcOffsetPGOOpTypeMap_ {};
    std::unordered_map<int32_t, const PGORWOpType*> bcOffsetPGORwTypeMap_ {};
    std::unordered_map<int32_t, const PGODefineOpType*> bcOffsetPGODefOpTypeMap_;
};
}  // panda::ecmascript::kungfu
#endif // ECMASCRIPT_COMPILER_PGO_TYPE_PGO_TYPE_RECORD_H
