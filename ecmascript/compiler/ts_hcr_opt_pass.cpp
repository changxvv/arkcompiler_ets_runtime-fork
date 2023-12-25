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

#include "ecmascript/compiler/ts_hcr_opt_pass.h"

namespace panda::ecmascript::kungfu {

GateRef TSHCROptPass::VisitGate(GateRef gate)
{
    auto opcode = acc_.GetOpCode(gate);
    switch (opcode) {
        case OpCode::TYPED_BINARY_OP:
            return VisitTypedBinaryOp(gate);
        default:
            break;
    }
    return Circuit::NullGate();
}

GateRef TSHCROptPass::VisitTypedBinaryOp(GateRef gate)
{
    if (acc_.HasStringType(gate)) {
        return VisitStringBinOp(gate);
    }
    return Circuit::NullGate();
}

GateRef TSHCROptPass::VisitStringBinOp(GateRef gate)
{
    TypedBinOp op = acc_.GetTypedBinaryOp(gate);
    switch (op) {
        case TypedBinOp::TYPED_EQ:
            return VisitStringEqual(gate);
        default:
            return Circuit::NullGate();
    }
}

GateRef TSHCROptPass::VisitStringEqual(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    GateRef left = acc_.GetValueIn(gate, 0);
    GateRef right = acc_.GetValueIn(gate, 1);
    if (acc_.IsConstString(left) && acc_.IsConstString(right)) {
        return ConvertStringEqualToConst(left, right);
    }

    if (IsSingleCharString(left) && IsSingleCharString(right)) {
        return ConvertToSingleCharComparison(left, right);
    }

    return Circuit::NullGate();
}

GateRef TSHCROptPass::ConvertStringEqualToConst(GateRef left, GateRef right)
{
    uint32_t leftId = acc_.GetStringIdFromLdaStrGate(left);
    uint32_t rightId = acc_.GetStringIdFromLdaStrGate(right);

    auto leftMethodOffset = acc_.TryGetMethodOffset(left);
    auto rightMethodOffset = acc_.TryGetMethodOffset(right);
    JSHandle<EcmaString> leftStr(thread_, GetStringFromCP(leftMethodOffset, leftId));
    JSHandle<EcmaString> rightStr(thread_, GetStringFromCP(rightMethodOffset, rightId));
    bool isEqual = EcmaStringAccessor::StringsAreEqual(thread_->GetEcmaVM(), leftStr, rightStr);
    if (isEqual) {
        return builder_.Boolean(true);
    }
    return builder_.Boolean(false);
}

bool TSHCROptPass::IsSingleCharString(GateRef gate)
{
    if (acc_.IsConstString(gate)) {
        uint32_t strId = acc_.GetStringIdFromLdaStrGate(gate);
        auto methodOffset = acc_.TryGetMethodOffset(gate);
        JSTaggedValue str = GetStringFromCP(methodOffset, strId);
        return EcmaStringAccessor(str).GetLength() == 1;
    }
    return acc_.IsSingleCharGate(gate);
}

GateRef TSHCROptPass::ConvertConstSingleCharToInt32(GateRef gate)
{
    ASSERT(acc_.IsConstString(gate));
    uint32_t strId = acc_.GetStringIdFromLdaStrGate(gate);
    auto methodOffset = acc_.TryGetMethodOffset(gate);
    JSTaggedValue str = tsManager_->GetStringFromConstantPool(methodOffset, strId);
    ASSERT(EcmaStringAccessor(str).GetLength() == 1);
    uint16_t strToInt = EcmaStringAccessor(str).Get(0);
    return builder_.Int32(strToInt);
}

GateRef TSHCROptPass::ConvertToSingleCharComparison(GateRef left, GateRef right)
{
    ASSERT(!acc_.IsConstString(left) || !acc_.IsConstString(right));
    if (acc_.IsConstString(left)) {
        left = ConvertConstSingleCharToInt32(left);
    } else if (acc_.IsConstString(right)) {
        right = ConvertConstSingleCharToInt32(right);
    }
    return builder_.TypedBinaryOp<TypedBinOp::TYPED_EQ>(left, right, GateType::IntType(),
                                                        GateType::IntType(), GateType::BooleanType(),
                                                        PGOTypeRef::NoneType());
}
}  // namespace panda::ecmascript::kungfu
