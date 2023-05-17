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

#include "ecmascript/compiler/async_function_lowering.h"

namespace panda::ecmascript::kungfu {
void AsyncFunctionLowering::ProcessAll()
{
    ProcessJumpTable();

    if (IsLogEnabled()) {
        LOG_COMPILER(INFO) << "";
        LOG_COMPILER(INFO) << "\033[34m"
                           << "===================="
                           << " After async function lowering "
                           << "[" << GetMethodName() << "]"
                           << "===================="
                           << "\033[0m";
        circuit_->PrintAllGatesWithBytecode();
        LOG_COMPILER(INFO) << "\033[34m" << "========================= End ==========================" << "\033[0m";
    }
}

void AsyncFunctionLowering::ProcessJumpTable()
{
    GateRef newTarget = argAccessor_.GetCommonArgGate(CommonArgIdx::NEW_TARGET);
    GateRef isEqual = builder_.Equal(newTarget, builder_.Undefined());
    auto firstUse = accessor_.ConstUses(stateEntry_).begin();
    GateRef ifBranchCondition = builder_.Branch(stateEntry_, isEqual);
    GateRef ifTrueCondition = builder_.IfTrue(ifBranchCondition);
    GateRef ifFalseCondition = builder_.IfFalse(ifBranchCondition);
    if (accessor_.GetOpCode(*firstUse) == OpCode::STATE_SPLIT) {
        firstUse++;
    }
    accessor_.ReplaceStateIn(*firstUse, ifTrueCondition);

    GateRef contextOffset = builder_.IntPtr(JSGeneratorObject::GENERATOR_CONTEXT_OFFSET);
    GateRef val = builder_.PtrAdd(newTarget, contextOffset);
    GateRef dependStart = builder_.DependRelay(ifFalseCondition, dependEntry_);
    GateRef contextGate = circuit_->NewGate(circuit_->Load(), MachineType::I64, {dependStart, val},
                                            GateType::TaggedPointer());
    GateRef bcOffset = builder_.IntPtr(GeneratorContext::GENERATOR_BC_OFFSET_OFFSET);
    val = builder_.PtrAdd(contextGate, bcOffset);
    GateRef restoreOffsetGate = circuit_->NewGate(circuit_->Load(), MachineType::I32, {contextGate, val},
                                                  GateType::NJSValue());
    GateRef firstState = Circuit::NullGate();
    const auto &suspendAndResumeGates = bcBuilder_->GetAsyncRelatedGates();
    for (const auto &gate : suspendAndResumeGates) {
        EcmaOpcode ecmaOpcode = accessor_.GetByteCodeOpcode(gate);
        if (ecmaOpcode == EcmaOpcode::RESUMEGENERATOR) {
            RebuildGeneratorCfg(gate, restoreOffsetGate, ifFalseCondition, newTarget, firstState);
        }
    }
}

void AsyncFunctionLowering::RebuildGeneratorCfg(GateRef resumeGate, GateRef restoreOffsetGate, GateRef ifFalseCondition,
                                                GateRef newTarget, GateRef &firstState)
{
    GateRef stateGate = accessor_.GetState(resumeGate);
    GateRef suspendGate = stateGate;
    if (accessor_.GetOpCode(suspendGate) == OpCode::IF_SUCCESS) {
        suspendGate = accessor_.GetState(suspendGate);
    }
    GateRef firstRestoreRegGate = GetFirstRestoreRegister(resumeGate);
    GateRef offsetConstantGate = accessor_.GetValueIn(suspendGate);
    offsetConstantGate = builder_.TruncInt64ToInt32(offsetConstantGate);
    auto stateInGate = accessor_.GetState(resumeGate);
    bool flag = true;
    GateRef prevLoopBeginGate = Circuit::NullGate();
    GateRef loopBeginStateIn = Circuit::NullGate();
    GateRef prevBcOffsetPhiGate = Circuit::NullGate();
    while (true) {
        auto opcode = accessor_.GetOpCode(stateInGate);
        if (opcode == OpCode::STATE_ENTRY) {
            GateRef condition = builder_.Equal(offsetConstantGate, restoreOffsetGate);
            GateRef ifBranch = circuit_->NewGate(circuit_->IfBranch(), { ifFalseCondition, condition });
            GateRef ifTrue = circuit_->NewGate(circuit_->IfTrue(), {ifBranch});
            GateRef ifFalse = circuit_->NewGate(circuit_->IfFalse(), {ifBranch});
            GateRef ifTrueDepend = builder_.DependRelay(ifTrue, restoreOffsetGate);
            GateRef ifFalseDepend = builder_.DependRelay(ifFalse, restoreOffsetGate);
            if (flag) {
                accessor_.ReplaceStateIn(resumeGate, ifTrue);
                accessor_.ReplaceValueIn(resumeGate, newTarget);
                accessor_.ReplaceDependIn(firstRestoreRegGate, ifTrueDepend);
                circuit_->NewGate(circuit_->Return(), MachineType::NOVALUE,
                    { stateGate, suspendGate, suspendGate, circuit_->GetReturnRoot() },
                    GateType::AnyType());
            } else {
                loopBeginStateIn = ifTrue;
            }
            accessor_.ReplaceStateIn(ifBranch, ifFalseCondition);
            if (firstState != Circuit::NullGate()) {
                accessor_.ReplaceStateIn(firstState, ifFalse);
            } else {
                auto constant = builder_.UndefineConstant();
                circuit_->NewGate(circuit_->Return(), MachineType::NOVALUE,
                    { ifFalse, ifFalseDepend, constant, circuit_->GetReturnRoot() },
                    GateType::AnyType());
            }
            firstState = ifBranch;
        }

        if (opcode == OpCode::LOOP_BEGIN) {
            bool resumeInLoopBody = false;
            CheckResumeInLoopBody(stateInGate, resumeInLoopBody);
            if (resumeInLoopBody) {
                // This constant gate must be created by the NewGate method to distinguish whether the while
                // loop needs to modify the phi node or not.
                GateRef emptyOffsetGate = circuit_->NewGate(circuit_->GetMetaBuilder()->Constant(-1),
                                                            MachineType::I32, GateType::NJSValue());
                // 2: valuesIn
                GateRef bcOffsetPhiGate = circuit_->NewGate(circuit_->ValueSelector(2), MachineType::I32,
                                                            {stateInGate, restoreOffsetGate, emptyOffsetGate},
                                                            GateType::NJSValue());

                GateRef condition = builder_.Equal(offsetConstantGate, bcOffsetPhiGate);
                GateRef ifBranch = circuit_->NewGate(circuit_->IfBranch(), {stateInGate, condition});
                GateRef ifTrue = circuit_->NewGate(circuit_->IfTrue(), {ifBranch});
                GateRef ifFalse = circuit_->NewGate(circuit_->IfFalse(), {ifBranch});

                GateRef resumeStateGate = accessor_.GetState(resumeGate);
                if (accessor_.GetOpCode(resumeStateGate) != OpCode::IF_TRUE) {
                    accessor_.ReplaceStateIn(resumeGate, ifTrue);
                    accessor_.ReplaceValueIn(resumeGate, newTarget);
                    accessor_.ReplaceDependIn(firstRestoreRegGate, GetDependPhiFromLoopBegin(stateInGate));
                    circuit_->NewGate(circuit_->Return(), MachineType::NOVALUE,
                        { stateGate, suspendGate, suspendGate, circuit_->GetReturnRoot() },
                        GateType::AnyType());
                } else {
                    // Handling multi-layer for loops
                    // When in a multi-layer loop, the value-selector node of the prev-loop
                    // should be used directly instead of generating a new node
                    UpdateValueSelector(prevLoopBeginGate, ifTrue, prevBcOffsetPhiGate, false);
                    accessor_.ReplaceValueIn(prevBcOffsetPhiGate, bcOffsetPhiGate);
                }
                accessor_.ReplaceStateIn(ifBranch, stateInGate);
                ModifyStateInput(stateInGate, ifBranch, ifFalse);

                prevLoopBeginGate = stateInGate;
                prevBcOffsetPhiGate = bcOffsetPhiGate;
                stateInGate = accessor_.GetState(stateInGate);
                flag = false;
                continue;
            }
        }
        if (loopBeginStateIn != Circuit::NullGate()) {
            UpdateValueSelector(prevLoopBeginGate, loopBeginStateIn, prevBcOffsetPhiGate);
            break;
        }
        if (accessor_.GetOpCode(stateInGate) == OpCode::STATE_ENTRY) {
            break;
        }
        stateInGate = accessor_.GetState(stateInGate);
    }
}

void AsyncFunctionLowering::UpdateValueSelector(GateRef prevLoopBeginGate,
                                                GateRef controlStateGate,
                                                GateRef prevBcOffsetPhiGate,
                                                bool genNewValuePhiGate)
{
    GateRef loopBeginFirstState = accessor_.GetState(prevLoopBeginGate);
    // 2: statesIn
    GateRef newGate = circuit_->NewGate(circuit_->Merge(2),
                                        {controlStateGate, loopBeginFirstState});

    if (genNewValuePhiGate) {
        GateRef emptyOffsetGate = circuit_->NewGate(circuit_->GetMetaBuilder()->Constant(-1), // -1: distinguish bcoffset
                                                    MachineType::I32, GateType::NJSValue());
        GateRef restoreOffset = accessor_.GetValueIn(prevBcOffsetPhiGate);
        // this value selector is compatible with await in the loop body
        GateRef valueSelector = circuit_->NewGate(circuit_->ValueSelector(2), MachineType::I32, // 2: num of valueIn
                                                  {newGate, restoreOffset, emptyOffsetGate},
                                                  GateType::NJSValue());
        accessor_.ReplaceValueIn(prevBcOffsetPhiGate, valueSelector);
    }
    accessor_.ReplaceStateIn(prevLoopBeginGate, newGate);
    auto loopBeginUses = accessor_.Uses(prevLoopBeginGate);
    for (auto use : loopBeginUses) {
        if (accessor_.GetOpCode(use) == OpCode::VALUE_SELECTOR && use != prevBcOffsetPhiGate) {
            auto machineType = accessor_.GetMachineType(use);
            auto gateType = accessor_.GetGateType(use);
            GateRef undefinedGate =
                circuit_->NewGate(circuit_->GetMetaBuilder()->Constant(JSTaggedValue::VALUE_UNDEFINED),
                                  machineType, gateType);
            auto firstValueGate = accessor_.GetValueIn(use, 0);
            auto newValueSelector = circuit_->NewGate(circuit_->ValueSelector(2), machineType, // 2: valuesIn
                                                      {newGate, undefinedGate, firstValueGate},
                                                      gateType);
            accessor_.ReplaceValueIn(use, newValueSelector);
        } else if (accessor_.GetOpCode(use) == OpCode::DEPEND_SELECTOR) {
            // if there is a dependSelector in the use node of the loop-begin, a new dependSelector node needs
            // to be generated. This node is bound to the merge node (newGate) before the loop-begin, and its
            // input corresponds to the 'dependEntry' (not the frist time enter the function) and
            // 'dependGate' (the first time enter the function) nodes.
            auto dependGate = accessor_.GetDep(use);
            auto newDependSelector = circuit_->NewGate(circuit_->DependSelector(2), // 2: num of dependIn
                                                       {newGate, circuit_->GetDependRoot(), dependGate});
            accessor_.ReplaceDependIn(use, newDependSelector);
        }
    }
}

bool AsyncFunctionLowering::IsAsyncRelated() const
{
    return bcBuilder_->GetAsyncRelatedGates().size() > 0;
}

void AsyncFunctionLowering::ModifyStateInput(GateRef stateInGate, GateRef ifBranch, GateRef ifFalse)
{
    // Find the node with LOOP_BEGIN as State input and modify its
    // state input to the newly created IF_FALSE node.
    auto uses = accessor_.Uses(stateInGate);
    for (auto useIt = uses.begin(); useIt != uses.end();) {
        GateRef use = *useIt;
        if (accessor_.IsState(use) && use != ifBranch) {
            useIt = accessor_.ReplaceIn(useIt, ifFalse);
        } else {
            useIt++;
        }
    }
}

void AsyncFunctionLowering::CheckResumeInLoopBody(GateRef stateInGate, bool &resumeInLoopBody)
{
    ASSERT(accessor_.GetOpCode(stateInGate) == OpCode::LOOP_BEGIN);
    GateRef loopBack = accessor_.GetIn(stateInGate, 0);
    if (accessor_.GetOpCode(loopBack) != OpCode::LOOP_BACK) {
        loopBack = accessor_.GetIn(stateInGate, 1);
    }
    ChunkQueue<GateRef> resuemList(circuit_->chunk());
    resuemList.push(loopBack);
    ChunkVector<VisitState> visited(circuit_->GetMaxGateId() + 1, VisitState::UNVISITED, circuit_->chunk());
    auto loopBeginId = accessor_.GetId(stateInGate);
    visited[loopBeginId] = VisitState::VISITED;
    auto loopBackId = accessor_.GetId(loopBack);
    visited[loopBackId] = VisitState::VISITED;
    while (!resuemList.empty()) {
        GateRef curGate = resuemList.front();
        if (accessor_.GetOpCode(curGate) == OpCode::JS_BYTECODE &&
            accessor_.GetByteCodeOpcode(curGate) == EcmaOpcode::RESUMEGENERATOR) {
            resumeInLoopBody = true;
            break;
        }
        resuemList.pop();
        size_t stateStart = 0;
        size_t stateEnd = accessor_.GetStateCount(curGate);
        for (size_t idx = stateStart; idx < stateEnd; idx++) {
            GateRef gate = accessor_.GetState(curGate, idx);
            auto id = accessor_.GetId(gate);
            if (visited[id] == VisitState::UNVISITED) {
                visited[id] = VisitState::VISITED;
                resuemList.push(gate);
            }
        }
    }
}

GateRef AsyncFunctionLowering::GetFirstRestoreRegister(GateRef gate) const
{
    GateRef firstRestoreGate = gate;
    GateRef curRestoreGate = accessor_.GetDep(gate);
    if (accessor_.GetOpCode(curRestoreGate) == OpCode::STATE_SPLIT) {
        curRestoreGate = accessor_.GetDep(curRestoreGate);
    }
    while (accessor_.GetOpCode(curRestoreGate) == OpCode::RESTORE_REGISTER) {
        firstRestoreGate = curRestoreGate;
        curRestoreGate = accessor_.GetDep(curRestoreGate);
    }
    return firstRestoreGate;
}

GateRef AsyncFunctionLowering::GetDependPhiFromLoopBegin(GateRef gate) const
{
    auto loopBeginUses = accessor_.ConstUses(gate);
    for (auto use : loopBeginUses) {
        if (accessor_.GetOpCode(use) == OpCode::DEPEND_SELECTOR) {
            return use;
        }
    }
    LOG_COMPILER(FATAL) << "Can not find depend-selector from loopbegin";
    return Circuit::NullGate();
}
}  // panda::ecmascript::kungfu

