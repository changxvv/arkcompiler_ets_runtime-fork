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

#ifndef ECMASCRIPT_COMPILER_TYPE_INFERENCE_PGO_TYPE_INFER_H
#define ECMASCRIPT_COMPILER_TYPE_INFERENCE_PGO_TYPE_INFER_H

#include "ecmascript/compiler/gate_accessor.h"
#include "ecmascript/ts_types/ts_manager.h"
#include "ecmascript/compiler/argument_accessor.h"

namespace panda::ecmascript::kungfu {
class PGOTypeInfer {
public:
    PGOTypeInfer(Circuit *circuit, TSManager *tsManager, BytecodeCircuitBuilder *builder,
                 const std::string &name, Chunk *chunk, bool enableLog)
        : circuit_(circuit), acc_(circuit), argAcc_(circuit), tsManager_(tsManager),
          builder_(builder), methodName_(name), chunk_(chunk), enableLog_(enableLog), profiler_(chunk) {}
    ~PGOTypeInfer() = default;

    void Run();

private:
    struct Profiler {
        struct Value {
            GateRef gate;
            GateType tsType;
            CVector<GateType> pgoTypes;
            CVector<GateType> inferTypes;
        };
        Profiler(Chunk *chunk) : datas(chunk) {}
        ChunkVector<Value> datas;
    };

    enum class RWOpLoc : uint8_t {
        UNKONWN = 0,
        INSTANCE,
        CONSTRUCTOR,
    };

    inline bool IsLogEnabled() const
    {
        return enableLog_;
    }

    inline const std::string &GetMethodName() const
    {
        return methodName_;
    }

    inline bool IsMonoTypes(const ChunkSet<GateType> &types) const
    {
        return types.size() == 1;
    }

    inline bool NoNeedUpdate(const ChunkSet<GateType> &types) const
    {
        return types.size() <= 1;
    }

    void RunTypeInfer(GateRef gate);
    void InferLdObjByName(GateRef gate);
    void InferStObjByName(GateRef gate, bool isThis);

    void UpdateTypeForRWOp(GateRef gate, GateRef receiver, JSTaggedValue prop);
    bool CheckPGOType(PGORWOpType pgoTypes, RWOpLoc &rwOpLoc) const;
    ChunkSet<GateType> UpdateType(GateType tsType, PGORWOpType pgoType, JSTaggedValue prop);
    void CollectGateType(ChunkSet<GateType> &types, GateType tsType, PGORWOpType pgoTypes);
    void CheckAndInsert(ChunkSet<GateType> &types, GateType type);
    void EliminateSubclassTypes(ChunkSet<GateType> &types);
    void ComputeCommonSuperClassTypes(ChunkSet<GateType> &types, JSTaggedValue prop);
    void Print() const;
    void AddProfiler(GateRef gate, GateType tsType, PGORWOpType pgoType, ChunkSet<GateType>& inferTypes);

    Circuit *circuit_ {nullptr};
    GateAccessor acc_;
    ArgumentAccessor argAcc_;
    TSManager *tsManager_ {nullptr};
    BytecodeCircuitBuilder *builder_ {nullptr};
    const std::string &methodName_;
    Chunk *chunk_ {nullptr};
    bool enableLog_ {false};
    Profiler profiler_;
};
}  // panda::ecmascript::kungfu
#endif  // ECMASCRIPT_COMPILER_TYPE_INFERENCE_PGO_TYPE_INFER_H
