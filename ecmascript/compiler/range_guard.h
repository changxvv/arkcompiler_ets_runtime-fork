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

#ifndef ECMASCRIPT_COMPILER_RANGE_GUARD_H
#define ECMASCRIPT_COMPILER_RANGE_GUARD_H

#include "ecmascript/compiler/circuit_builder.h"
#include "ecmascript/compiler/gate_accessor.h"
#include "ecmascript/compiler/graph_visitor.h"
#include "ecmascript/compiler/pass_manager.h"
#include "ecmascript/compiler/base/depend_chain_helper.h"
#include "ecmascript/mem/chunk_containers.h"

namespace panda::ecmascript::kungfu {
class DependChains;
class RangeGuard : public GraphVisitor {
public:
    RangeGuard(Circuit *circuit, Chunk* chunk)
        : GraphVisitor(circuit, chunk), circuit_(circuit),
        builder_(circuit), dependChains_(chunk) {}

    ~RangeGuard() = default;

    void Run();

    GateRef VisitGate(GateRef gate) override;
    bool CheckInputSource(GateRef lhs, GateRef rhs);
    bool CheckIndexCheckLengthInput(GateRef lhs, GateRef rhs);
    bool CheckIndexCheckIndexInput(GateRef lhs, GateRef rhs);
private:

    DependChains* GetDependChain(GateRef dependIn)
    {
        size_t idx = acc_.GetId(dependIn);
        ASSERT(idx <= circuit_->GetMaxGateId());
        return dependChains_[idx];
    }

    GateRef VisitDependEntry(GateRef gate);
    GateRef UpdateDependChain(GateRef gate, DependChains* dependInfo);
    GateRef TryApplyTypedArrayRangeGuardForLength(DependChains* dependInfo, GateRef gate, GateRef input);
    GateRef TryApplyTypedArrayRangeGuardForIndex(DependChains* dependInfo, GateRef gate, GateRef input);
    GateRef TryApplyRangeGuardGate(GateRef gate);
    GateRef TraverseOthers(GateRef gate);
    GateRef TraverseDependSelector(GateRef gate);

    Circuit* circuit_;
    CircuitBuilder builder_;
    ChunkVector<DependChains*> dependChains_;
};
}  // panda::ecmascript::kungfu
#endif  // ECMASCRIPT_COMPILER_RANGE_GUARD_H