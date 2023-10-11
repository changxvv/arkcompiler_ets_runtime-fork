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

#include "ecmascript/compiler/circuit_builder-inl.h"
#include "ecmascript/compiler/mcr_circuit_builder.h"

namespace panda::ecmascript::kungfu {

GateRef CircuitBuilder::ObjectTypeCheck(GateType type, bool isHeapObject, GateRef gate, GateRef hclassIndex)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto frameState = acc_.FindNearestFrameState(currentDepend);
    ObjectTypeAccessor accessor(type, isHeapObject);
    GateRef ret = GetCircuit()->NewGate(circuit_->ObjectTypeCheck(accessor.ToValue()), MachineType::I1,
        {currentControl, currentDepend, gate, hclassIndex, frameState}, GateType::NJSValue());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::ObjectTypeCompare(GateType type, bool isHeapObject, GateRef gate, GateRef hclassIndex)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto frameState = acc_.FindNearestFrameState(currentDepend);
    ObjectTypeAccessor accessor(type, isHeapObject);
    GateRef ret = GetCircuit()->NewGate(circuit_->ObjectTypeCompare(accessor.ToValue()), MachineType::I1,
        {currentControl, currentDepend, gate, hclassIndex, frameState}, GateType::NJSValue());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::HeapObjectCheck(GateRef gate, GateRef frameState)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    GateRef ret = GetCircuit()->NewGate(circuit_->HeapObjectCheck(),
                                        MachineType::I1,
                                        {currentControl, currentDepend, gate, frameState},
                                        GateType::NJSValue());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::StableArrayCheck(GateRef gate, ElementsKind kind, ArrayMetaDataAccessor::Mode mode)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto frameState = acc_.FindNearestFrameState(currentDepend);
    ArrayMetaDataAccessor accessor(kind, mode);
    GateRef ret = GetCircuit()->NewGate(circuit_->StableArrayCheck(accessor.ToValue()),
        MachineType::I1, {currentControl, currentDepend, gate, frameState}, GateType::NJSValue());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::COWArrayCheck(GateRef gate)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto frameState = acc_.FindNearestFrameState(currentDepend);
    GateRef ret = GetCircuit()->NewGate(circuit_->COWArrayCheck(),
        MachineType::I1, {currentControl, currentDepend, gate, frameState}, GateType::NJSValue());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::EcmaStringCheck(GateRef gate)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto frameState = acc_.FindNearestFrameState(currentDepend);
    GateRef ret = GetCircuit()->NewGate(circuit_->EcmaStringCheck(),
        MachineType::I1, {currentControl, currentDepend, gate, frameState}, GateType::NJSValue());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::FlattenTreeStringCheck(GateRef gate)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto frameState = acc_.FindNearestFrameState(currentDepend);
    GateRef ret = GetCircuit()->NewGate(circuit_->FlattenTreeStringCheck(),
        MachineType::I1, {currentControl, currentDepend, gate, frameState}, GateType::NJSValue());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::HClassStableArrayCheck(GateRef gate, GateRef frameState, ArrayMetaDataAccessor accessor)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    GateRef ret = GetCircuit()->NewGate(circuit_->HClassStableArrayCheck(accessor.ToValue()),
        MachineType::I1, {currentControl, currentDepend, gate, frameState}, GateType::NJSValue());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::ArrayGuardianCheck(GateRef frameState)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    GateRef ret = GetCircuit()->NewGate(circuit_->ArrayGuardianCheck(),
        MachineType::I1, {currentControl, currentDepend, frameState}, GateType::NJSValue());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::TypedArrayCheck(GateType type, GateRef gate)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto frameState = acc_.FindNearestFrameState(currentDepend);
    GateRef ret = GetCircuit()->NewGate(circuit_->TypedArrayCheck(static_cast<size_t>(type.Value())),
        MachineType::I1, {currentControl, currentDepend, gate, frameState}, GateType::NJSValue());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::LoadTypedArrayLength(GateType type, GateRef gate)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    GateRef ret = GetCircuit()->NewGate(circuit_->LoadTypedArrayLength(static_cast<size_t>(type.Value())),
        MachineType::I64, {currentControl, currentDepend, gate}, GateType::IntType());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::StringEqual(GateRef x, GateRef y)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto ret = GetCircuit()->NewGate(circuit_->StringEqual(), MachineType::I1,
                                     { currentControl, currentDepend, x, y }, GateType::BooleanType());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::RangeGuard(GateRef gate, uint32_t left, uint32_t right)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    UInt32PairAccessor accessor(left, right);
    GateRef ret = GetCircuit()->NewGate(circuit_->RangeGuard(accessor.ToValue()),
        MachineType::I64, {currentControl, currentDepend, gate}, GateType::IntType());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::IndexCheck(GateType type, GateRef gate, GateRef index)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto frameState = acc_.FindNearestFrameState(currentDepend);
    GateRef ret = GetCircuit()->NewGate(circuit_->IndexCheck(static_cast<size_t>(type.Value())),
        MachineType::I64, {currentControl, currentDepend, gate, index, frameState}, GateType::IntType());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::TypeOfCheck(GateRef gate, GateType type)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto frameState = acc_.FindNearestFrameState(currentDepend);
    GateRef ret = GetCircuit()->NewGate(circuit_->TypeOfCheck(static_cast<size_t>(type.Value())),
        MachineType::I64, {currentControl, currentDepend, gate, frameState}, GateType::IntType());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::TypedTypeOf(GateType type)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    GateRef ret = GetCircuit()->NewGate(circuit_->TypeOf(static_cast<size_t>(type.Value())),
        MachineType::I64, {currentControl, currentDepend}, GateType::AnyType());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::CheckAndConvert(GateRef gate, ValueType src, ValueType dst, ConvertSupport support)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto stateSplit = acc_.FindNearestStateSplit(currentDepend);
    auto frameState = acc_.GetFrameState(stateSplit);
    MachineType machineType = GetMachineTypeOfValueType(dst);
    GateType gateType = GetGateTypeOfValueType(dst);
    uint64_t value = ValuePairTypeAccessor::ToValue(src, dst, support);
    GateRef ret = GetCircuit()->NewGate(circuit_->CheckAndConvert(value),
        machineType, {currentControl, currentDepend, gate, frameState}, gateType);
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::Convert(GateRef gate, ValueType src, ValueType dst)
{
    MachineType machineType = GetMachineTypeOfValueType(dst);
    GateType gateType = GetGateTypeOfValueType(dst);
    uint64_t value = ValuePairTypeAccessor::ToValue(src, dst);
    GateRef ret = GetCircuit()->NewGate(circuit_->Convert(value), machineType, {gate}, gateType);
    return ret;
}

GateRef CircuitBuilder::ConvertBoolToInt32(GateRef gate, ConvertSupport support)
{
    return CheckAndConvert(gate, ValueType::BOOL, ValueType::INT32, support);
}

GateRef CircuitBuilder::ConvertBoolToFloat64(GateRef gate, ConvertSupport support)
{
    return CheckAndConvert(gate, ValueType::BOOL, ValueType::FLOAT64, support);
}

GateRef CircuitBuilder::ConvertCharToEcmaString(GateRef gate)
{
    return Convert(gate, ValueType::CHAR, ValueType::ECMA_STRING);
}

GateRef CircuitBuilder::ConvertInt32ToFloat64(GateRef gate)
{
    return Convert(gate, ValueType::INT32, ValueType::FLOAT64);
}

GateRef CircuitBuilder::ConvertUInt32ToFloat64(GateRef gate)
{
    return Convert(gate, ValueType::UINT32, ValueType::FLOAT64);
}

GateRef CircuitBuilder::ConvertFloat64ToInt32(GateRef gate)
{
    return Convert(gate, ValueType::FLOAT64, ValueType::INT32);
}

GateRef CircuitBuilder::ConvertBoolToTaggedBoolean(GateRef gate)
{
    return Convert(gate, ValueType::BOOL, ValueType::TAGGED_BOOLEAN);
}

GateRef CircuitBuilder::ConvertTaggedBooleanToBool(GateRef gate)
{
    return Convert(gate, ValueType::TAGGED_BOOLEAN, ValueType::BOOL);
}

GateRef CircuitBuilder::ConvertInt32ToTaggedInt(GateRef gate)
{
    return Convert(gate, ValueType::INT32, ValueType::TAGGED_INT);
}

GateRef CircuitBuilder::ConvertUInt32ToTaggedNumber(GateRef gate)
{
    return Convert(gate, ValueType::UINT32, ValueType::TAGGED_NUMBER);
}

GateRef CircuitBuilder::ConvertInt32ToBool(GateRef gate)
{
    return Convert(gate, ValueType::INT32, ValueType::BOOL);
}

GateRef CircuitBuilder::ConvertUInt32ToBool(GateRef gate)
{
    return Convert(gate, ValueType::UINT32, ValueType::BOOL);
}

GateRef CircuitBuilder::ConvertFloat64ToBool(GateRef gate)
{
    return Convert(gate, ValueType::FLOAT64, ValueType::BOOL);
}

GateRef CircuitBuilder::CheckTaggedBooleanAndConvertToBool(GateRef gate)
{
    return CheckAndConvert(gate, ValueType::TAGGED_BOOLEAN, ValueType::BOOL);
}

GateRef CircuitBuilder::CheckTaggedNumberAndConvertToBool(GateRef gate)
{
    return CheckAndConvert(gate, ValueType::TAGGED_NUMBER, ValueType::BOOL);
}

GateRef CircuitBuilder::ConvertFloat64ToTaggedDouble(GateRef gate)
{
    return Convert(gate, ValueType::FLOAT64, ValueType::TAGGED_DOUBLE);
}

GateRef CircuitBuilder::CheckUInt32AndConvertToInt32(GateRef gate)
{
    return CheckAndConvert(gate, ValueType::UINT32, ValueType::INT32);
}

GateRef CircuitBuilder::CheckTaggedIntAndConvertToInt32(GateRef gate)
{
    return CheckAndConvert(gate, ValueType::TAGGED_INT, ValueType::INT32);
}

GateRef CircuitBuilder::CheckTaggedDoubleAndConvertToInt32(GateRef gate)
{
    return CheckAndConvert(gate, ValueType::TAGGED_DOUBLE, ValueType::INT32);
}

GateRef CircuitBuilder::CheckTaggedNumberAndConvertToInt32(GateRef gate)
{
    return CheckAndConvert(gate, ValueType::TAGGED_NUMBER, ValueType::INT32);
}

GateRef CircuitBuilder::CheckTaggedIntAndConvertToFloat64(GateRef gate)
{
    return CheckAndConvert(gate, ValueType::TAGGED_INT, ValueType::FLOAT64);
}

GateRef CircuitBuilder::CheckTaggedDoubleAndConvertToFloat64(GateRef gate)
{
    return CheckAndConvert(gate, ValueType::TAGGED_DOUBLE, ValueType::FLOAT64);
}

GateRef CircuitBuilder::CheckTaggedNumberAndConvertToFloat64(GateRef gate)
{
    return CheckAndConvert(gate, ValueType::TAGGED_NUMBER, ValueType::FLOAT64);
}

GateRef CircuitBuilder::CheckNullAndConvertToInt32(GateRef gate)
{
    return CheckAndConvert(gate, ValueType::TAGGED_NULL, ValueType::INT32);
}

GateRef CircuitBuilder::CheckTaggedBooleanAndConvertToInt32(GateRef gate)
{
    return CheckAndConvert(gate, ValueType::TAGGED_BOOLEAN, ValueType::INT32);
}

GateRef CircuitBuilder::CheckNullAndConvertToFloat64(GateRef gate)
{
    return CheckAndConvert(gate, ValueType::TAGGED_NULL, ValueType::FLOAT64);
}

GateRef CircuitBuilder::CheckTaggedBooleanAndConvertToFloat64(GateRef gate)
{
    return CheckAndConvert(gate, ValueType::TAGGED_BOOLEAN, ValueType::FLOAT64);
}

GateRef CircuitBuilder::CheckUndefinedAndConvertToFloat64(GateRef gate)
{
    return CheckAndConvert(gate, ValueType::UNDEFINED, ValueType::FLOAT64);
}

GateRef CircuitBuilder::CheckUndefinedAndConvertToBool(GateRef gate)
{
    return CheckAndConvert(gate, ValueType::UNDEFINED, ValueType::BOOL);
}

GateRef CircuitBuilder::CheckNullAndConvertToBool(GateRef gate)
{
    return CheckAndConvert(gate, ValueType::TAGGED_NULL, ValueType::BOOL);
}

GateRef CircuitBuilder::CheckUndefinedAndConvertToInt32(GateRef gate)
{
    return CheckAndConvert(gate, ValueType::UNDEFINED, ValueType::INT32);
}

GateRef CircuitBuilder::TryPrimitiveTypeCheck(GateType type, GateRef gate)
{
    if (acc_.GetOpCode(gate) == OpCode::CONSTANT) {
        return Circuit::NullGate();
    }
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto frameState = acc_.FindNearestFrameState(currentDepend);
    GateRef ret = GetCircuit()->NewGate(circuit_->PrimitiveTypeCheck(static_cast<size_t>(type.Value())),
        MachineType::I1, {currentControl, currentDepend, gate, frameState}, GateType::NJSValue());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::CallTargetCheck(GateRef gate, GateRef function, GateRef id, GateRef param, const char* comment)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto frameState = acc_.GetFrameState(gate);
    GateRef ret = GetCircuit()->NewGate(circuit_->TypedCallCheck(),
                                        MachineType::I1,
                                        { currentControl, currentDepend, function, id, param, frameState},
                                        GateType::NJSValue(),
                                        comment);
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::JSCallTargetFromDefineFuncCheck(GateType type, GateRef func, GateRef gate)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto frameState = acc_.GetFrameState(gate);
    GateRef ret = GetCircuit()->NewGate(circuit_->TypedCallTargetCheckOp(1, static_cast<size_t>(type.Value()),
        TypedCallTargetCheckOp::JSCALL_IMMEDIATE_AFTER_FUNC_DEF),
        MachineType::I1, {currentControl, currentDepend, func, frameState}, GateType::NJSValue());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}
GateRef CircuitBuilder::TypedCallOperator(GateRef hirGate, MachineType type, const std::vector<GateRef> &inList)
{
    ASSERT(acc_.GetOpCode(hirGate) == OpCode::JS_BYTECODE);
    auto numValueIn = inList.size() - 3; // 3: state & depend & frame state
    uint64_t pcOffset = acc_.TryGetPcOffset(hirGate);
    ASSERT(pcOffset != 0);
    return GetCircuit()->NewGate(circuit_->TypedCallBuiltin(numValueIn, pcOffset), type, inList.size(), inList.data(),
                                 GateType::AnyType());
}


GateRef CircuitBuilder::TypedNewAllocateThis(GateRef ctor, GateRef hclassIndex, GateRef frameState)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    GateRef ret = GetCircuit()->NewGate(circuit_->TypedNewAllocateThis(),
        MachineType::ANYVALUE, {currentControl, currentDepend, ctor, hclassIndex, frameState}, GateType::TaggedValue());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::TypedSuperAllocateThis(GateRef superCtor, GateRef newTarget, GateRef frameState)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    GateRef ret = GetCircuit()->NewGate(circuit_->TypedSuperAllocateThis(), MachineType::ANYVALUE,
        {currentControl, currentDepend, superCtor, newTarget, frameState}, GateType::TaggedValue());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}


GateRef CircuitBuilder::Int32CheckRightIsZero(GateRef right)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto frameState = acc_.FindNearestFrameState(currentDepend);
    GateRef ret = GetCircuit()->NewGate(circuit_->Int32CheckRightIsZero(),
    MachineType::I1, {currentControl, currentDepend, right, frameState}, GateType::NJSValue());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::Float64CheckRightIsZero(GateRef right)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto frameState = acc_.FindNearestFrameState(currentDepend);
    GateRef ret = GetCircuit()->NewGate(circuit_->Float64CheckRightIsZero(),
    MachineType::I1, {currentControl, currentDepend, right, frameState}, GateType::NJSValue());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::LexVarIsHoleCheck(GateRef value)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto frameState = acc_.FindNearestFrameState(currentDepend);
    GateRef ret = GetCircuit()->NewGate(circuit_->LexVarIsHoleCheck(),
    MachineType::I1, {currentControl, currentDepend, value, frameState}, GateType::NJSValue());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::ValueCheckNegOverflow(GateRef value)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto frameState = acc_.FindNearestFrameState(currentDepend);
    GateRef ret = GetCircuit()->NewGate(circuit_->ValueCheckNegOverflow(),
    MachineType::I1, {currentControl, currentDepend, value, frameState}, GateType::NJSValue());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::OverflowCheck(GateRef value)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto frameState = acc_.FindNearestFrameState(currentDepend);
    GateRef ret = GetCircuit()->NewGate(circuit_->OverflowCheck(),
        MachineType::I1, {currentControl, currentDepend, value, frameState}, GateType::IntType());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::Int32UnsignedUpperBoundCheck(GateRef value, GateRef upperBound)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto frameState = acc_.FindNearestFrameState(currentDepend);
    GateRef ret = GetCircuit()->NewGate(circuit_->Int32UnsignedUpperBoundCheck(),
        MachineType::I1, {currentControl, currentDepend, value, upperBound, frameState}, GateType::IntType());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::Int32DivWithCheck(GateRef left, GateRef right)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto frameState = acc_.FindNearestFrameState(currentDepend);
    GateRef ret = GetCircuit()->NewGate(circuit_->Int32DivWithCheck(),
        MachineType::I32, {currentControl, currentDepend, left, right, frameState}, GateType::NJSValue());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::TypedConditionJump(MachineType type, TypedJumpOp jumpOp, uint32_t weight,
    GateType typeVal, const std::vector<GateRef>& inList)
{
    uint64_t value = TypedJumpAccessor::ToValue(typeVal, jumpOp, weight);
    return GetCircuit()->NewGate(circuit_->TypedConditionJump(value),
        type, inList.size(), inList.data(), GateType::Empty());
}

GateRef CircuitBuilder::TypeConvert(MachineType type, GateType typeFrom, GateType typeTo,
                                    const std::vector<GateRef>& inList)
{
    // merge types of valueIns before and after convertion
    uint64_t operandTypes = GatePairTypeAccessor::ToValue(typeFrom, typeTo);
    return GetCircuit()->NewGate(circuit_->TypedConvert(operandTypes),
        type, inList.size(), inList.data(), GateType::AnyType());
}

GateRef CircuitBuilder::StoreMemory(MemoryType Op, VariableType type, GateRef receiver, GateRef index, GateRef value)
{
    auto opIdx = static_cast<uint64_t>(Op);
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto ret = GetCircuit()->NewGate(GetCircuit()->StoreMemory(opIdx), type.GetMachineType(),
        {currentControl, currentDepend, receiver, index, value}, type.GetGateType());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::LoadProperty(GateRef receiver, GateRef propertyLookupResult, bool isFunction)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto ret = GetCircuit()->NewGate(circuit_->LoadProperty(isFunction), MachineType::I64,
                                     { currentControl, currentDepend, receiver, propertyLookupResult },
                                     GateType::AnyType());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::StoreProperty(GateRef receiver, GateRef propertyLookupResult, GateRef value)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto ret = GetCircuit()->NewGate(circuit_->StoreProperty(), MachineType::I64,
                                     { currentControl, currentDepend, receiver, propertyLookupResult, value },
                                     GateType::AnyType());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::LoadArrayLength(GateRef array)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto ret = GetCircuit()->NewGate(circuit_->LoadArrayLength(), MachineType::I64,
                                     { currentControl, currentDepend, array }, GateType::IntType());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::LoadStringLength(GateRef string)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto ret = GetCircuit()->NewGate(circuit_->LoadStringLength(), MachineType::I64,
                                     { currentControl, currentDepend, string }, GateType::IntType());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::LoadConstOffset(VariableType type, GateRef receiver, size_t offset)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentDepend = currentLabel->GetDepend();

    auto ret = GetCircuit()->NewGate(circuit_->LoadConstOffset(offset), type.GetMachineType(),
                                     { currentDepend, receiver }, type.GetGateType());
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::StoreConstOffset(VariableType type,
    GateRef receiver, size_t offset, GateRef value)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();

    auto ret = GetCircuit()->NewGate(circuit_->StoreConstOffset(offset), type.GetMachineType(),
        { currentControl, currentDepend, receiver, value }, type.GetGateType());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::ConvertHoleAsUndefined(GateRef receiver)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();

    auto ret = GetCircuit()->NewGate(circuit_->ConvertHoleAsUndefined(),
        MachineType::I64, { currentControl, currentDepend, receiver }, GateType::AnyType());
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::TypedCall(GateRef hirGate, std::vector<GateRef> args, bool isNoGC)
{
    ASSERT(acc_.GetOpCode(hirGate) == OpCode::JS_BYTECODE);
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    uint64_t bitfield = args.size();
    uint64_t pcOffset = acc_.TryGetPcOffset(hirGate);
    ASSERT(pcOffset != 0);
    args.insert(args.begin(), currentDepend);
    args.insert(args.begin(), currentControl);
    AppendFrameArgs(args, hirGate);
    auto callGate = GetCircuit()->NewGate(circuit_->TypedCall(bitfield, pcOffset, isNoGC), MachineType::I64,
                                          args.size(), args.data(), GateType::AnyType());
    currentLabel->SetControl(callGate);
    currentLabel->SetDepend(callGate);
    return callGate;
}

GateRef CircuitBuilder::TypedFastCall(GateRef hirGate, std::vector<GateRef> args, bool isNoGC)
{
    ASSERT(acc_.GetOpCode(hirGate) == OpCode::JS_BYTECODE);
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    uint64_t bitfield = args.size();
    uint64_t pcOffset = acc_.TryGetPcOffset(hirGate);
    ASSERT(pcOffset != 0);
    args.insert(args.begin(), currentDepend);
    args.insert(args.begin(), currentControl);
    AppendFrameArgs(args, hirGate);
    auto callGate = GetCircuit()->NewGate(circuit_->TypedFastCall(bitfield, pcOffset, isNoGC), MachineType::I64,
                                          args.size(), args.data(), GateType::AnyType());
    currentLabel->SetControl(callGate);
    currentLabel->SetDepend(callGate);
    return callGate;
}

GateRef CircuitBuilder::StartAllocate()
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentDepend = currentLabel->GetDepend();
    GateRef newGate = GetCircuit()->NewGate(circuit_->StartAllocate(),  MachineType::I64,
                                            { currentDepend }, GateType::NJSValue());
    currentLabel->SetDepend(newGate);
    return newGate;
}

GateRef CircuitBuilder::FinishAllocate()
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentDepend = currentLabel->GetDepend();
    GateRef newGate = GetCircuit()->NewGate(circuit_->FinishAllocate(),  MachineType::I64,
                                            { currentDepend }, GateType::NJSValue());
    currentLabel->SetDepend(newGate);
    return newGate;
}

GateRef CircuitBuilder::HeapAlloc(GateRef size, GateType type, RegionSpaceFlag flag)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto ret = GetCircuit()->NewGate(circuit_->HeapAlloc(flag), MachineType::ANYVALUE,
                                     { currentControl, currentDepend, size }, type);
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateType CircuitBuilder::GetGateTypeOfValueType(ValueType type)
{
    switch (type) {
        case ValueType::BOOL:
        case ValueType::INT32:
        case ValueType::FLOAT64:
            return GateType::NJSValue();
        case ValueType::TAGGED_BOOLEAN:
            return GateType::BooleanType();
        case ValueType::TAGGED_INT:
            return GateType::IntType();
        case ValueType::TAGGED_DOUBLE:
            return GateType::DoubleType();
        case ValueType::TAGGED_NUMBER:
            return GateType::NumberType();
        default:
            return GateType::Empty();
    }
}

GateRef CircuitBuilder::InsertTypedBinaryop(GateRef left, GateRef right, GateType leftType, GateType rightType,
                                            GateType gateType, PGOSampleType sampleType, TypedBinOp op)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    uint64_t operandTypes = GatePairTypeAccessor::ToValue(leftType, rightType);
    auto ret = GetCircuit()->NewGate(circuit_->TypedBinaryOp(operandTypes, op, sampleType),
                                     MachineType::I64,
                                     {currentControl, currentDepend, left, right},
                                     gateType);
    acc_.ReplaceInAfterInsert(currentControl, currentDepend, ret);
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::InsertRangeCheckPredicate(GateRef left, TypedBinOp cond, GateRef right)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    auto frameState = acc_.FindNearestFrameState(currentDepend);
    TypedBinaryAccessor accessor(GateType::IntType(), cond);
    auto ret = GetCircuit()->NewGate(circuit_->RangeCheckPredicate(accessor.ToValue()),
                                     MachineType::I32,
                                     {currentControl, currentDepend, left, right, frameState},
                                     GateType::IntType());
    acc_.ReplaceInAfterInsert(currentControl, currentDepend, ret);
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::InsertStableArrayCheck(GateRef array)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    GateRef frameState = acc_.FindNearestFrameState(currentDepend);
    ElementsKind kind = acc_.TryGetElementsKind(array);
    ArrayMetaDataAccessor::Mode mode = ArrayMetaDataAccessor::Mode::LOAD_LENGTH;
    ArrayMetaDataAccessor accessor(kind, mode);
    auto ret = GetCircuit()->NewGate(circuit_->StableArrayCheck(accessor.ToValue()),
                                     MachineType::I1,
                                     {currentControl, currentDepend, array, frameState},
                                     GateType::NJSValue());
    acc_.ReplaceInAfterInsert(currentControl, currentDepend, ret);
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::InsertTypedArrayCheck(GateType type, GateRef array)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    GateRef frameState = acc_.FindNearestFrameState(currentDepend);
    auto ret = GetCircuit()->NewGate(circuit_->TypedArrayCheck(static_cast<size_t>(type.Value())),
                                     MachineType::I1,
                                     {currentControl, currentDepend, array, frameState},
                                     GateType::NJSValue());
    acc_.ReplaceInAfterInsert(currentControl, currentDepend, ret);
    currentLabel->SetControl(ret);
    currentLabel->SetDepend(ret);
    return ret;
}

GateRef CircuitBuilder::InsertLoadArrayLength(GateRef array, bool isTypedArray)
{
    auto currentLabel = env_->GetCurrentLabel();
    auto currentControl = currentLabel->GetControl();
    auto currentDepend = currentLabel->GetDepend();
    GateType arrayType = acc_.GetGateType(array);
    if (isTypedArray) {
        InsertTypedArrayCheck(arrayType, array);
        currentControl = currentLabel->GetControl();
        currentDepend = currentLabel->GetDepend();
        auto ret = GetCircuit()->NewGate(circuit_->LoadTypedArrayLength(static_cast<size_t>(arrayType.Value())),
                                         MachineType::I64,
                                         { currentControl, currentDepend, array },
                                         GateType::IntType());
        acc_.ReplaceInAfterInsert(currentControl, currentDepend, ret);
        currentLabel->SetControl(ret);
        currentLabel->SetDepend(ret);
        return ret;
    } else {
        InsertStableArrayCheck(array);
        currentControl = currentLabel->GetControl();
        currentDepend = currentLabel->GetDepend();
        auto ret = GetCircuit()->NewGate(circuit_->LoadArrayLength(),
                                         MachineType::I64,
                                         { currentControl, currentDepend, array },
                                         GateType::IntType());
        acc_.ReplaceInAfterInsert(currentControl, currentDepend, ret);
        currentLabel->SetControl(ret);
        currentLabel->SetDepend(ret);
        return ret;
    }
    UNREACHABLE();
    return Circuit::NullGate();
}

GateRef CircuitBuilder::GetLengthFromString(GateRef value)
{
    GateRef len = Load(VariableType::INT32(), value, IntPtr(EcmaString::MIX_LENGTH_OFFSET));
    return Int32LSR(len, Int32(EcmaString::STRING_LENGTH_SHIFT_COUNT));
}

GateRef CircuitBuilder::GetHashcodeFromString(GateRef glue, GateRef value)
{
    ASSERT(!GetCircuit()->IsOptimizedJSFunctionFrame());
    Label subentry(env_);
    SubCfgEntry(&subentry);
    Label noRawHashcode(env_);
    Label exit(env_);
    DEFVAlUE(hashcode, env_, VariableType::INT32(), Int32(0));
    hashcode = Load(VariableType::INT32(), value, IntPtr(EcmaString::HASHCODE_OFFSET));
    Branch(Int32Equal(*hashcode, Int32(0)), &noRawHashcode, &exit);
    Bind(&noRawHashcode);
    {
        hashcode = GetInt32OfTInt(
            CallRuntime(glue, RTSTUB_ID(ComputeHashcode), Gate::InvalidGateRef, { value }, Circuit::NullGate()));
        Store(VariableType::INT32(), glue, value, IntPtr(EcmaString::HASHCODE_OFFSET), *hashcode);
        Jump(&exit);
    }
    Bind(&exit);
    auto ret = *hashcode;
    SubCfgExit();
    return ret;
}

GateRef CircuitBuilder::TryGetHashcodeFromString(GateRef string)
{
    Label subentry(env_);
    SubCfgEntry(&subentry);
    Label noRawHashcode(env_);
    Label storeHash(env_);
    Label exit(env_);
    DEFVAlUE(result, env_, VariableType::INT64(), Int64(-1));
    GateRef hashCode = ZExtInt32ToInt64(Load(VariableType::INT32(), string, IntPtr(EcmaString::HASHCODE_OFFSET)));
    Branch(Int64Equal(hashCode, Int64(0)), &noRawHashcode, &storeHash);
    Bind(&noRawHashcode);
    {
        GateRef length = GetLengthFromString(string);
        Label lengthNotZero(env_);
        Branch(Int32Equal(length, Int32(0)), &storeHash, &exit);
    }
    Bind(&storeHash);
    result = hashCode;
    Jump(&exit);
    Bind(&exit);
    auto ret = *result;
    SubCfgExit();
    return ret;
}

GateRef CircuitBuilder::ComputeTaggedArraySize(GateRef length)
{
    return PtrAdd(IntPtr(TaggedArray::DATA_OFFSET),
        PtrMul(IntPtr(JSTaggedValue::TaggedTypeSize()), length));
}
}