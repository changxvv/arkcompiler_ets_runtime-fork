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

#ifndef ECMASCRIPT_COMPILER_NUMBER_SPECULATIVE_RETYPE_H
#define ECMASCRIPT_COMPILER_NUMBER_SPECULATIVE_RETYPE_H

#include "ecmascript/compiler/circuit_builder.h"
#include "ecmascript/compiler/gate_accessor.h"
#include "ecmascript/compiler/gate_meta_data.h"
#include "ecmascript/compiler/graph_visitor.h"
#include "ecmascript/compiler/number_gate_info.h"
#include "ecmascript/compiler/type.h"
#include "ecmascript/mem/chunk_containers.h"

namespace panda::ecmascript::kungfu {
class NumberSpeculativeRetype : public GraphVisitor {
public:
    NumberSpeculativeRetype(Circuit *circuit, Chunk* chunk, ChunkVector<TypeInfo>& typeInfos)
        : GraphVisitor(circuit, chunk), acc_(circuit), builder_(circuit), typeInfos_(typeInfos) {}
    void Run();
    GateRef VisitGate(GateRef gate);
    
private:
    enum class State {
        Retype,
        Convert,
    };

    bool IsRetype() const
    {
        return state_ == State::Retype;
    }
    
    bool IsConvert() const
    {
        return state_ == State::Convert;
    }
    
    GateRef SetOutputType(GateRef gate, GateType type);
    GateRef VisitPhi(GateRef gate);
    GateRef VisitConstant(GateRef gate);
    GateRef VisitNumberBinaryOp(GateRef gate);
    GateRef VisitNumberCalculate(GateRef gate);
    GateRef VisitNumberCompare(GateRef gate);
    void ConvertForIntOperator(GateRef gate);
    void ConvertForDoubleOperator(GateRef gate);
    GateRef VisitNumberRelated(GateRef gate);
    GateRef VisitOthers(GateRef gate);

    GateRef CheckAndConvertToInt32(GateRef gate, GateType gateType);
    GateRef CheckAndConvertToFloat64(GateRef gate, GateType gateType);
    GateRef CheckAndConvertToTagged(GateRef gate, GateType gateType);
    GateRef ConvertToTagged(GateRef gate);

    GateAccessor acc_;
    CircuitBuilder builder_;
    ChunkVector<TypeInfo>& typeInfos_;
    State state_ {0};
};
}  // panda::ecmascript::kungfu
#endif  // ECMASCRIPT_COMPILER_NUMBER_SPECULATIVE_RETYPE_H