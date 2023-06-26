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

#ifndef ECMASCRIPT_COMPILER_BUILTIN_LOWERING_H
#define ECMASCRIPT_COMPILER_BUILTIN_LOWERING_H

#include <string>
#include "ecmascript/compiler/argument_accessor.h"
#include "ecmascript/compiler/builtins/builtins_call_signature.h"
#include "ecmascript/compiler/circuit_builder-inl.h"

namespace panda::ecmascript::kungfu {
class BuiltinLowering {
public:
    explicit BuiltinLowering(Circuit *circuit): circuit_(circuit), builder_(circuit), acc_(circuit) {}
    ~BuiltinLowering() = default;
    void LowerTypedCallBuitin(GateRef gate);
    GateRef LowerCallTargetCheck(Environment *env, GateRef gate);
    void LowerTypedSqrt(GateRef gate);
    GateRef CheckPara(GateRef gate, GateRef funcCheck);
    void LowerTypedLocaleCompare(GateRef gate);

private:
    void LowerTypedTrigonometric(GateRef gate, BuiltinsStubCSigns::ID id);
    GateRef TypedTrigonometric(GateRef gate, BuiltinsStubCSigns::ID id);
    GateRef IntToTaggedIntPtr(GateRef x);
    void LowerTypedAbs(GateRef gate);
    GateRef TypedAbs(GateRef gate);
    GateRef LowerCallRuntime(GateRef glue, GateRef gate, int index, const std::vector<GateRef> &args,
                             bool useLabel = false);
    void ReplaceHirWithValue(GateRef hirGate, GateRef value, bool noThrow = false);

    Circuit *circuit_ {nullptr};
    CircuitBuilder builder_;
    GateAccessor acc_;
};
}  // panda::ecmascript::kungfu
#endif  // ECMASCRIPT_COMPILER_BUILTIN_LOWERING_H
