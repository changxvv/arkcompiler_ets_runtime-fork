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

#ifndef ECMASCRIPT_COMPILER_NTYPE_BYTECODE_LOWERING_H
#define ECMASCRIPT_COMPILER_NTYPE_BYTECODE_LOWERING_H

#include "ecmascript/compiler/argument_accessor.h"
#include "ecmascript/compiler/builtins/builtins_call_signature.h"
#include "ecmascript/compiler/bytecode_circuit_builder.h"
#include "ecmascript/compiler/circuit_builder-inl.h"
#include "ecmascript/compiler/pass_manager.h"

namespace panda::ecmascript::kungfu {
class NTypeBytecodeLowering {
public:
    NTypeBytecodeLowering(Circuit *circuit, PassContext *ctx, TSManager *tsManager, const MethodLiteral *methodLiteral,
                     const CString &recordName, bool enableLog, const std::string& name)
        : circuit_(circuit),
          acc_(circuit),
          builder_(circuit, ctx->GetCompilerConfig()),
          recordName_(recordName),
          tsManager_(tsManager),
          jsPandaFile_(ctx->GetJSPandaFile()),
          methodLiteral_(methodLiteral),
          enableLog_(enableLog),
          profiling_(ctx->GetCompilerConfig()->IsProfiling()),
          traceBc_(ctx->GetCompilerConfig()->IsTraceBC()),
          methodName_(name),
          glue_(acc_.GetGlueFromArgList()) {}

    ~NTypeBytecodeLowering() = default;

    void RunNTypeBytecodeLowering();
private:
    void Lower(GateRef gate);
    void LowerNTypedCreateEmptyArray(GateRef gate);
    void LowerNTypedCreateArrayWithBuffer(GateRef gate);
    void LowerNTypedStownByIndex(GateRef gate);
    void LowerNTypedStOwnByName(GateRef gate);
    void LowerLdLexVar(GateRef gate);
    void LowerStLexVar(GateRef gate);
    void LowerThrowUndefinedIfHoleWithName(GateRef gate);
    uint64_t GetBcAbsoluteOffset(GateRef gate) const;

    bool IsLogEnabled() const
    {
        return enableLog_;
    }

    bool IsProfiling() const
    {
        return profiling_;
    }

    bool IsTraceBC() const
    {
        return traceBc_;
    }

    const std::string& GetMethodName() const
    {
        return methodName_;
    }

    void AddProfiling(GateRef gate);
    Circuit *circuit_ {nullptr};
    GateAccessor acc_;
    CircuitBuilder builder_;
    const CString &recordName_;
    TSManager *tsManager_ {nullptr};
    const JSPandaFile *jsPandaFile_ {nullptr};
    const MethodLiteral *methodLiteral_ {nullptr};
    bool enableLog_ {false};
    bool profiling_ {false};
    bool traceBc_ {false};
    std::string methodName_;
    GateRef glue_ {Circuit::NullGate()};
};
}  // panda::ecmascript::kungfu
#endif  // ECMASCRIPT_COMPILER_NTYPE_BYTECODE_LOWERING_H
