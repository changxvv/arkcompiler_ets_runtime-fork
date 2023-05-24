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

#include "ecmascript/compiler/type_mcr_lowering.h"
#include "ecmascript/compiler/builtins_lowering.h"
#include "ecmascript/compiler/new_object_stub_builder.h"
#include "ecmascript/deoptimizer/deoptimizer.h"
#include "ecmascript/js_arraybuffer.h"
#include "ecmascript/js_native_pointer.h"
#include "ecmascript/subtyping_operator.h"
#include "ecmascript/vtable.h"

namespace panda::ecmascript::kungfu {
void TypeMCRLowering::RunTypeMCRLowering()
{
    std::vector<GateRef> gateList;
    circuit_->GetAllGates(gateList);
    for (const auto &gate : gateList) {
        LowerType(gate);
    }

    if (IsLogEnabled()) {
        LOG_COMPILER(INFO) << "";
        LOG_COMPILER(INFO) << "\033[34m" << "=================="
                           << " after TypeMCRlowering "
                           << "[" << GetMethodName() << "] "
                           << "==================" << "\033[0m";
        circuit_->PrintAllGatesWithBytecode();
        LOG_COMPILER(INFO) << "\033[34m" << "=========================== End =========================" << "\033[0m";
    }
}

void TypeMCRLowering::LowerType(GateRef gate)
{
    GateRef glue = acc_.GetGlueFromArgList();
    auto op = acc_.GetOpCode(gate);
    switch (op) {
        case OpCode::PRIMITIVE_TYPE_CHECK:
            LowerPrimitiveTypeCheck(gate);
            break;
        case OpCode::STABLE_ARRAY_CHECK:
            LowerStableArrayCheck(gate);
            break;
        case OpCode::TYPED_ARRAY_CHECK:
            LowerTypedArrayCheck(gate);
            break;
        case OpCode::OBJECT_TYPE_CHECK:
            LowerObjectTypeCheck(gate);
            break;
        case OpCode::INDEX_CHECK:
            LowerIndexCheck(gate);
            break;
        case OpCode::JSCALLTARGET_TYPE_CHECK:
            LowerJSCallTargetTypeCheck(gate);
            break;
        case OpCode::JSFASTCALLTARGET_TYPE_CHECK:
            LowerJSFastCallTargetTypeCheck(gate);
            break;
        case OpCode::JSCALLTHISTARGET_TYPE_CHECK:
            LowerJSCallThisTargetTypeCheck(gate);
            break;
        case OpCode::JSFASTCALLTHISTARGET_TYPE_CHECK:
            LowerJSFastCallThisTargetTypeCheck(gate);
            break;
        case OpCode::TYPED_CALL_CHECK:
            LowerCallTargetCheck(gate);
            break;
        case OpCode::JSINLINETARGET_TYPE_CHECK:
            LowerJSInlineTargetTypeCheck(gate);
            break;
        case OpCode::TYPED_BINARY_OP:
            LowerTypedBinaryOp(gate);
            break;
        case OpCode::TYPE_CONVERT:
            LowerTypeConvert(gate);
            break;
        case OpCode::LOAD_PROPERTY:
            LowerLoadProperty(gate);
            break;
        case OpCode::CALL_GETTER:
            LowerCallGetter(gate, glue);
            break;
        case OpCode::STORE_PROPERTY:
        case OpCode::STORE_PROPERTY_NO_BARRIER:
            LowerStoreProperty(gate);
            break;
        case OpCode::CALL_SETTER:
            LowerCallSetter(gate, glue);
            break;
        case OpCode::LOAD_ARRAY_LENGTH:
            LowerLoadArrayLength(gate);
            break;
        case OpCode::LOAD_ELEMENT:
            LowerLoadElement(gate);
            break;
        case OpCode::STORE_ELEMENT:
            LowerStoreElement(gate, glue);
            break;
        case OpCode::HEAP_ALLOC:
            LowerHeapAllocate(gate, glue);
            break;
        case OpCode::TYPED_CALL_BUILTIN:
            LowerTypedCallBuitin(gate);
            break;
        case OpCode::TYPED_NEW_ALLOCATE_THIS:
            LowerTypedNewAllocateThis(gate, glue);
            break;
        case OpCode::TYPED_SUPER_ALLOCATE_THIS:
            LowerTypedSuperAllocateThis(gate, glue);
            break;
        case OpCode::GET_SUPER_CONSTRUCTOR:
            LowerGetSuperConstructor(gate);
            break;
        default:
            break;
    }
}

void TypeMCRLowering::LowerPrimitiveTypeCheck(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    auto type = acc_.GetParamGateType(gate);
    if (type.IsIntType()) {
        LowerIntCheck(gate);
    } else if (type.IsDoubleType()) {
        LowerDoubleCheck(gate);
    } else if (type.IsNumberType()) {
        LowerNumberCheck(gate);
    } else if (type.IsBooleanType()) {
        LowerBooleanCheck(gate);
    } else {
        LOG_ECMA(FATAL) << "this branch is unreachable";
        UNREACHABLE();
    }
}

void TypeMCRLowering::LowerIntCheck(GateRef gate)
{
    GateRef frameState = GetFrameState(gate);

    GateRef value = acc_.GetValueIn(gate, 0);
    GateRef typeCheck = builder_.TaggedIsInt(value);
    builder_.DeoptCheck(typeCheck, frameState, DeoptType::NOTINT);

    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), Circuit::NullGate());
}

void TypeMCRLowering::LowerDoubleCheck(GateRef gate)
{
    GateRef frameState = GetFrameState(gate);

    GateRef value = acc_.GetValueIn(gate, 0);
    GateRef typeCheck = builder_.TaggedIsDouble(value);
    builder_.DeoptCheck(typeCheck, frameState, DeoptType::NOTDOUBLE);

    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), Circuit::NullGate());
}

void TypeMCRLowering::LowerNumberCheck(GateRef gate)
{
    GateRef frameState = GetFrameState(gate);

    GateRef value = acc_.GetValueIn(gate, 0);
    GateRef typeCheck = builder_.TaggedIsNumber(value);
    builder_.DeoptCheck(typeCheck, frameState, DeoptType::NOTNUMBER);

    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), Circuit::NullGate());
}

void TypeMCRLowering::LowerBooleanCheck(GateRef gate)
{
    GateRef frameState = GetFrameState(gate);

    GateRef value = acc_.GetValueIn(gate, 0);
    GateRef typeCheck = builder_.TaggedIsBoolean(value);
    builder_.DeoptCheck(typeCheck, frameState, DeoptType::NOTBOOL);

    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), Circuit::NullGate());
}

void TypeMCRLowering::LowerStableArrayCheck(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    GateRef frameState = GetFrameState(gate);

    GateRef receiver = acc_.GetValueIn(gate, 0);
    builder_.HeapObjectCheck(receiver, frameState);

    GateRef receiverHClass = builder_.LoadConstOffset(
        VariableType::JS_POINTER(), receiver, TaggedObject::HCLASS_OFFSET);
    builder_.HClassStableArrayCheck(receiverHClass, frameState);
    builder_.ArrayGuardianCheck(frameState);

    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), Circuit::NullGate());
}

void TypeMCRLowering::LowerTypedArrayCheck(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    auto type = acc_.GetParamGateType(gate);
    if (tsManager_->IsFloat32ArrayType(type)) {
        LowerFloat32ArrayCheck(gate);
    } else {
        LOG_ECMA(FATAL) << "this branch is unreachable";
        UNREACHABLE();
    }
}

void TypeMCRLowering::LowerFloat32ArrayCheck(GateRef gate)
{
    GateRef frameState = GetFrameState(gate);

    GateRef glueGlobalEnv = builder_.GetGlobalEnv();
    GateRef receiver = acc_.GetValueIn(gate, 0);
    GateRef receiverHClass = builder_.LoadHClass(receiver);
    GateRef protoOrHclass = builder_.GetGlobalEnvObjHClass(glueGlobalEnv, GlobalEnv::FLOAT32_ARRAY_FUNCTION_INDEX);
    GateRef check = builder_.Equal(receiverHClass, protoOrHclass);
    builder_.DeoptCheck(check, frameState, DeoptType::NOTF32ARRAY);

    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), Circuit::NullGate());
}

void TypeMCRLowering::LowerObjectTypeCheck(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    auto type = acc_.GetParamGateType(gate);
    if (tsManager_->IsClassInstanceTypeKind(type)) {
        LowerTSSubtypingCheck(gate);
    } else {
        LOG_ECMA(FATAL) << "this branch is unreachable";
        UNREACHABLE();
    }
}

void TypeMCRLowering::LowerTSSubtypingCheck(GateRef gate)
{
    GateRef frameState = GetFrameState(gate);
    GateRef receiver = acc_.GetValueIn(gate, 0);
    builder_.HeapObjectCheck(receiver, frameState);

    GateRef aotHCIndex = acc_.GetValueIn(gate, 1);
    ArgumentAccessor argAcc(circuit_);
    GateRef jsFunc = argAcc.GetFrameArgsIn(frameState, FrameArgIdx::FUNC);
    DEFVAlUE(check, (&builder_), VariableType::BOOL(), builder_.False());
    JSTaggedValue aotHC = tsManager_->GetHClassFromCache(acc_.TryGetValue(aotHCIndex));
    ASSERT(aotHC.IsJSHClass());

    int32_t level = JSHClass::Cast(aotHC.GetTaggedObject())->GetLevel();
    ASSERT(level >= 0);

    GateRef receiverHClass = builder_.LoadConstOffset(
        VariableType::JS_POINTER(), receiver, TaggedObject::HCLASS_OFFSET);
    GateRef supers = LoadSupers(receiverHClass);

    auto hclassIndex = acc_.GetConstantValue(aotHCIndex);
    GateRef aotHCGate = LoadFromConstPool(jsFunc, hclassIndex);

    if (LIKELY(static_cast<uint32_t>(level) < SubtypingOperator::DEFAULT_SUPERS_CAPACITY)) {
        check = builder_.Equal(aotHCGate, GetValueFromSupers(supers, level));
    } else {
        GateRef levelGate = builder_.Int32(level);
        GateRef length = GetLengthFromSupers(supers);

        Label levelValid(&builder_);
        Label exit(&builder_);
        builder_.Branch(builder_.Int32LessThan(levelGate, length), &levelValid, &exit);
        builder_.Bind(&levelValid);
        {
            check = builder_.Equal(aotHCGate, GetValueFromSupers(supers, level));
            builder_.Jump(&exit);
        }
        builder_.Bind(&exit);
    }
    builder_.DeoptCheck(*check, frameState, DeoptType::INCONSISTENTHCLASS);

    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), Circuit::NullGate());
}

void TypeMCRLowering::LowerIndexCheck(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    auto type = acc_.GetParamGateType(gate);
    if (tsManager_->IsArrayTypeKind(type)) {
        LowerArrayIndexCheck(gate);
    } else if (tsManager_->IsFloat32ArrayType(type)) {
        LowerFloat32ArrayIndexCheck(gate);
    } else {
        LOG_ECMA(FATAL) << "this branch is unreachable";
        UNREACHABLE();
    }
}

void TypeMCRLowering::LowerArrayIndexCheck(GateRef gate)
{
    GateRef frameState = GetFrameState(gate);

    GateRef receiver = acc_.GetValueIn(gate, 0);
    GateRef hclassIndex = acc_.GetValueIn(gate, 1);
    GateRef length = acc_.GetGateType(receiver).IsNJSValueType() ? receiver :
            builder_.Load(VariableType::INT32(), receiver, builder_.IntPtr(JSArray::LENGTH_OFFSET));
    GateRef lengthCheck = builder_.Int32UnsignedLessThan(hclassIndex, length);
    GateRef nonNegativeCheck = builder_.Int32LessThanOrEqual(builder_.Int32(0), hclassIndex);
    GateRef check = builder_.BoolAnd(lengthCheck, nonNegativeCheck);
    builder_.DeoptCheck(check, frameState, DeoptType::NOTARRAYIDX);

    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), hclassIndex);
}

void TypeMCRLowering::LowerFloat32ArrayIndexCheck(GateRef gate)
{
    GateRef frameState = GetFrameState(gate);

    GateRef receiver = acc_.GetValueIn(gate, 0);
    GateRef index = acc_.GetValueIn(gate, 1);
    GateRef length = acc_.GetGateType(receiver).IsNJSValueType() ? receiver :
            builder_.Load(VariableType::INT32(), receiver, builder_.IntPtr(JSTypedArray::ARRAY_LENGTH_OFFSET));
    GateRef nonNegativeCheck = builder_.Int32LessThanOrEqual(builder_.Int32(0), index);
    GateRef lengthCheck = builder_.Int32UnsignedLessThan(index, length);
    GateRef check = builder_.BoolAnd(nonNegativeCheck, lengthCheck);
    builder_.DeoptCheck(check, frameState, DeoptType::NOTF32ARRAYIDX);

    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), index);
}

GateRef TypeMCRLowering::LowerCallRuntime(GateRef glue, GateRef hirGate, int index, const std::vector<GateRef> &args,
                                          bool useLabel)
{
    if (useLabel) {
        GateRef result = builder_.CallRuntime(glue, index, Gate::InvalidGateRef, args, hirGate);
        return result;
    } else {
        const CallSignature *cs = RuntimeStubCSigns::Get(RTSTUB_ID(CallRuntime));
        GateRef target = builder_.IntPtr(index);
        GateRef result = builder_.Call(cs, glue, target, dependEntry_, args, hirGate);
        return result;
    }
}

void TypeMCRLowering::LowerTypeConvert(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    auto leftType = acc_.GetLeftType(gate);
    auto rightType = acc_.GetRightType(gate);
    if (rightType.IsNumberType()) {
        GateRef value = acc_.GetValueIn(gate, 0);
        if (leftType.IsDigitablePrimitiveType()) {
            LowerPrimitiveToNumber(gate, value, leftType);
        }
        return;
    }
}

void TypeMCRLowering::LowerPrimitiveToNumber(GateRef dst, GateRef src, GateType srcType)
{
    Label exit(&builder_);
    DEFVAlUE(result, (&builder_), VariableType::JS_ANY(), builder_.HoleConstant());
    if (srcType.IsBooleanType()) {
        Label isTrue(&builder_);
        Label isFalse(&builder_);
        builder_.Branch(builder_.TaggedIsTrue(src), &isTrue, &isFalse);
        builder_.Bind(&isTrue);
        result = IntToTaggedIntPtr(builder_.Int32(1));
        builder_.Jump(&exit);
        builder_.Bind(&isFalse);
        result = IntToTaggedIntPtr(builder_.Int32(0));
        builder_.Jump(&exit);
    } else if (srcType.IsUndefinedType()) {
        result = DoubleToTaggedDoublePtr(builder_.Double(base::NAN_VALUE));
    } else if (srcType.IsBigIntType() || srcType.IsNumberType()) {
        result = src;
        builder_.Jump(&exit);
    } else if (srcType.IsNullType()) {
        result = IntToTaggedIntPtr(builder_.Int32(0));
        builder_.Jump(&exit);
    } else {
        LOG_ECMA(FATAL) << "this branch is unreachable";
        UNREACHABLE();
    }
    builder_.Bind(&exit);
    acc_.ReplaceGate(dst, builder_.GetState(), builder_.GetDepend(), *result);
}

GateRef TypeMCRLowering::LoadFromConstPool(GateRef jsFunc, size_t index)
{
    GateRef constPool = builder_.GetConstPool(jsFunc);
    return LoadFromTaggedArray(constPool, index);
}

GateRef TypeMCRLowering::GetObjectFromConstPool(GateRef jsFunc, GateRef index)
{
    GateRef constPool = builder_.GetConstPool(jsFunc);
    return builder_.GetValueFromTaggedArray(constPool, index);
}

void TypeMCRLowering::LowerLoadProperty(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    ASSERT(acc_.GetNumValueIn(gate) == 2);  // 2: receiver, plr
    GateRef receiver = acc_.GetValueIn(gate, 0);
    GateRef propertyLookupResult = acc_.GetValueIn(gate, 1);
    PropertyLookupResult plr(acc_.TryGetValue(propertyLookupResult));
    ASSERT(plr.IsLocal() || plr.IsFunction());

    GateRef result = Circuit::NullGate();
    if (plr.IsLocal()) {
        result = builder_.LoadConstOffset(VariableType::JS_ANY(), receiver, plr.GetOffset());
        result = builder_.ConvertHoleAsUndefined(result);
    } else {
        result = LoadFromVTable(receiver, plr.GetOffset());
    }

    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), result);
}

void TypeMCRLowering::LowerCallGetter(GateRef gate, GateRef glue)
{
    Environment env(gate, circuit_, &builder_);
    ASSERT(acc_.GetNumValueIn(gate) == 2);  // 2: receiver, plr
    GateRef receiver = acc_.GetValueIn(gate, 0);
    GateRef propertyLookupResult = acc_.GetValueIn(gate, 1);

    PropertyLookupResult plr(acc_.TryGetValue(propertyLookupResult));
    ASSERT(plr.IsAccessor());
    GateRef accessor = LoadFromVTable(receiver, plr.GetOffset());
    GateRef getter = builder_.Load(VariableType::JS_ANY(), accessor,
                                   builder_.IntPtr(AccessorData::GETTER_OFFSET));

    DEFVAlUE(result, (&builder_), VariableType::JS_ANY(), builder_.UndefineConstant());
    Label callGetter(&builder_);
    Label exit(&builder_);
    builder_.Branch(builder_.IsSpecial(getter, JSTaggedValue::VALUE_UNDEFINED), &exit, &callGetter);
    builder_.Bind(&callGetter);
    {
        result = CallAccessor(glue, gate, getter, receiver, AccessorMode::GETTER);
        builder_.Jump(&exit);
    }
    builder_.Bind(&exit);
    ReplaceHirWithPendingException(gate, glue, builder_.GetState(), builder_.GetDepend(), *result);
}

void TypeMCRLowering::LowerStoreProperty(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    ASSERT(acc_.GetNumValueIn(gate) == 3);  // 3: receiver, plr, value
    GateRef receiver = acc_.GetValueIn(gate, 0);
    GateRef propertyLookupResult = acc_.GetValueIn(gate, 1);
    GateRef value = acc_.GetValueIn(gate, 2); // 2: value
    PropertyLookupResult plr(acc_.TryGetValue(propertyLookupResult));
    ASSERT(plr.IsLocal());
    auto op = OpCode(acc_.GetOpCode(gate));
    if (op == OpCode::STORE_PROPERTY) {
        builder_.StoreConstOffset(VariableType::JS_ANY(), receiver, plr.GetOffset(), value);
    } else if (op == OpCode::STORE_PROPERTY_NO_BARRIER) {
        builder_.StoreConstOffset(VariableType::INT64(), receiver, plr.GetOffset(), value);
    } else {
        UNREACHABLE();
    }
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), Circuit::NullGate());
}

void TypeMCRLowering::LowerCallSetter(GateRef gate, GateRef glue)
{
    Environment env(gate, circuit_, &builder_);
    ASSERT(acc_.GetNumValueIn(gate) == 3);  // 3: receiver, plr, value
    GateRef receiver = acc_.GetValueIn(gate, 0);
    GateRef propertyLookupResult = acc_.GetValueIn(gate, 1);
    GateRef value = acc_.GetValueIn(gate, 2);

    PropertyLookupResult plr(acc_.TryGetValue(propertyLookupResult));
    ASSERT(plr.IsAccessor());
    GateRef accessor = LoadFromVTable(receiver, plr.GetOffset());
    GateRef setter = builder_.Load(VariableType::JS_ANY(),
        accessor, builder_.IntPtr(AccessorData::SETTER_OFFSET));

    Label callSetter(&builder_);
    Label exit(&builder_);
    builder_.Branch(builder_.IsSpecial(setter, JSTaggedValue::VALUE_UNDEFINED), &exit, &callSetter);
    builder_.Bind(&callSetter);
    {
        CallAccessor(glue, gate, setter, receiver, AccessorMode::SETTER, value);
        builder_.Jump(&exit);
    }
    builder_.Bind(&exit);
    ReplaceHirWithPendingException(gate, glue, builder_.GetState(), builder_.GetDepend(), Circuit::NullGate());
}

void TypeMCRLowering::LowerLoadArrayLength(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    GateRef array = acc_.GetValueIn(gate, 0);
    GateRef offset = builder_.IntPtr(JSArray::LENGTH_OFFSET);
    GateRef result = builder_.Load(VariableType::INT32(), array, offset);
    acc_.SetGateType(gate, GateType::NJSValue());
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), result);
}

void TypeMCRLowering::LowerLoadElement(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    auto op = acc_.GetTypedLoadOp(gate);
    switch (op) {
        case TypedLoadOp::ARRAY_LOAD_ELEMENT:
            LowerArrayLoadElement(gate);
            break;
        case TypedLoadOp::FLOAT32ARRAY_LOAD_ELEMENT:
            LowerFloat32ArrayLoadElement(gate);
            break;
        default:
            LOG_ECMA(FATAL) << "this branch is unreachable";
            UNREACHABLE();
    }
}

// for JSArray
void TypeMCRLowering::LowerArrayLoadElement(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    GateRef receiver = acc_.GetValueIn(gate, 0);
    GateRef index = acc_.GetValueIn(gate, 1);
    GateRef element = builder_.LoadConstOffset(
        VariableType::JS_POINTER(), receiver, JSObject::ELEMENTS_OFFSET);
    GateRef result = builder_.GetValueFromTaggedArray(element, index);
    result = builder_.ConvertHoleAsUndefined(result);
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), result);
}

// for Float32Array
void TypeMCRLowering::LowerFloat32ArrayLoadElement(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    Label isArrayBuffer(&builder_);
    Label isByteArray(&builder_);
    Label exit(&builder_);
    DEFVAlUE(res, (&builder_), VariableType::FLOAT32(), builder_.Double(0));
    GateRef receiver = acc_.GetValueIn(gate, 0);
    GateRef arrbuffer =
        builder_.Load(VariableType::JS_POINTER(), receiver, builder_.IntPtr(JSTypedArray::VIEWED_ARRAY_BUFFER_OFFSET));
    GateRef index = acc_.GetValueIn(gate, 1);
    GateRef elementSize = builder_.Int32(4);  // 4: float32 occupy 4 bytes
    GateRef offset = builder_.PtrMul(index, elementSize);
    GateRef byteOffset =
        builder_.Load(VariableType::INT32(), receiver, builder_.IntPtr(JSTypedArray::BYTE_OFFSET_OFFSET));
    // viewed arraybuffer could be JSArrayBuffer or ByteArray
    GateRef isOnHeap = builder_.Load(VariableType::BOOL(), receiver, builder_.IntPtr(JSTypedArray::ON_HEAP_OFFSET));
    builder_.Branch(isOnHeap, &isByteArray, &isArrayBuffer);
    builder_.Bind(&isByteArray);
    {
        GateRef data = builder_.PtrAdd(arrbuffer, builder_.IntPtr(ByteArray::DATA_OFFSET));
        res = builder_.Load(VariableType::FLOAT32(), data, builder_.PtrAdd(offset, byteOffset));
        builder_.Jump(&exit);
    }
    builder_.Bind(&isArrayBuffer);
    {
        GateRef data = builder_.Load(VariableType::JS_POINTER(), arrbuffer,
                                     builder_.IntPtr(JSArrayBuffer::DATA_OFFSET));
        GateRef block = builder_.Load(VariableType::JS_ANY(), data, builder_.IntPtr(JSNativePointer::POINTER_OFFSET));
        res = builder_.Load(VariableType::FLOAT32(), block, builder_.PtrAdd(offset, byteOffset));
        builder_.Jump(&exit);
    }

    builder_.Bind(&exit);
    GateRef result = builder_.Float32ToTaggedDoublePtr(*res);
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), result);
}

void TypeMCRLowering::LowerStoreElement(GateRef gate, GateRef glue)
{
    Environment env(gate, circuit_, &builder_);
    auto op = acc_.GetTypedStoreOp(gate);
    switch (op) {
        case TypedStoreOp::ARRAY_STORE_ELEMENT:
            LowerArrayStoreElement(gate, glue);
            break;
        case TypedStoreOp::FLOAT32ARRAY_STORE_ELEMENT:
            LowerFloat32ArrayStoreElement(gate, glue);
            break;
        default:
            LOG_ECMA(FATAL) << "this branch is unreachable";
            UNREACHABLE();
    }
}

// for JSArray
void TypeMCRLowering::LowerArrayStoreElement(GateRef gate, GateRef glue)
{
    Environment env(gate, circuit_, &builder_);
    GateRef receiver = acc_.GetValueIn(gate, 0);    // 0: receiver
    GateRef index = acc_.GetValueIn(gate, 1);   // 1: index
    GateRef value = acc_.GetValueIn(gate, 2);   // 2: value
    GateRef element = builder_.LoadConstOffset(
        VariableType::JS_POINTER(), receiver, JSObject::ELEMENTS_OFFSET);
    builder_.SetValueToTaggedArray(VariableType::JS_ANY(), glue, element, index, value);
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), Circuit::NullGate());
}

// for Float32Array
void TypeMCRLowering::LowerFloat32ArrayStoreElement(GateRef gate, GateRef glue)
{
    Environment env(gate, circuit_, &builder_);
    Label isArrayBuffer(&builder_);
    Label isByteArray(&builder_);
    Label afterGetValue(&builder_);
    Label exit(&builder_);
    GateRef receiver = acc_.GetValueIn(gate, 0);
    GateRef index = acc_.GetValueIn(gate, 1);
    GateRef elementSize = builder_.Int32(4);  // 4: float32 occupy 4 bytes
    GateRef offset = builder_.PtrMul(index, elementSize);
    GateRef byteOffset =
        builder_.Load(VariableType::INT32(), receiver, builder_.IntPtr(JSTypedArray::BYTE_OFFSET_OFFSET));

    GateRef value = acc_.GetValueIn(gate, 2);
    GateType valueType = acc_.GetGateType(value);
    DEFVAlUE(storeValue, (&builder_), VariableType::FLOAT32(), builder_.Double(0));
    if (valueType.IsIntType()) {
        storeValue = builder_.TaggedIntPtrToFloat32(value);
    } else if (valueType.IsDoubleType()) {
        storeValue = builder_.TaggedDoublePtrToFloat32(value);
    } else {
        Label valueIsInt(&builder_);
        Label valueIsDouble(&builder_);
        builder_.Branch(builder_.TaggedIsInt(value), &valueIsInt, &valueIsDouble);
        builder_.Bind(&valueIsInt);
        {
            storeValue = builder_.TaggedIntPtrToFloat32(value);
            builder_.Jump(&afterGetValue);
        }
        builder_.Bind(&valueIsDouble);
        {
            storeValue = builder_.TaggedDoublePtrToFloat32(value);
            builder_.Jump(&afterGetValue);
        }
        builder_.Bind(&afterGetValue);
    }
    GateRef arrbuffer =
        builder_.Load(VariableType::JS_POINTER(), receiver, builder_.IntPtr(JSTypedArray::VIEWED_ARRAY_BUFFER_OFFSET));
    // viewed arraybuffer could be JSArrayBuffer or ByteArray
    GateRef isOnHeap = builder_.Load(VariableType::BOOL(), receiver, builder_.IntPtr(JSTypedArray::ON_HEAP_OFFSET));
    builder_.Branch(isOnHeap, &isByteArray, &isArrayBuffer);
    builder_.Bind(&isByteArray);
    {
        GateRef data = builder_.PtrAdd(arrbuffer, builder_.IntPtr(ByteArray::DATA_OFFSET));
        builder_.Store(VariableType::VOID(), glue, data, builder_.PtrAdd(offset, byteOffset), *storeValue);
        builder_.Jump(&exit);
    }
    builder_.Bind(&isArrayBuffer);
    {
        GateRef data = builder_.Load(VariableType::JS_POINTER(), arrbuffer,
                                     builder_.IntPtr(JSArrayBuffer::DATA_OFFSET));
        GateRef block = builder_.Load(VariableType::JS_ANY(), data, builder_.IntPtr(JSNativePointer::POINTER_OFFSET));
        builder_.Store(VariableType::VOID(), glue, block, builder_.PtrAdd(offset, byteOffset), *storeValue);
        builder_.Jump(&exit);
    }
    builder_.Bind(&exit);
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), Circuit::NullGate());
}

void TypeMCRLowering::LowerHeapAllocate(GateRef gate, GateRef glue)
{
    Environment env(gate, circuit_, &builder_);
    auto bit = acc_.TryGetValue(gate);
    switch (bit) {
        case RegionSpaceFlag::IN_YOUNG_SPACE:
            LowerHeapAllocateInYoung(gate, glue);
            break;
        default:
            LOG_ECMA(FATAL) << "this branch is unreachable";
            UNREACHABLE();
    }
}

void TypeMCRLowering::LowerHeapAllocateInYoung(GateRef gate, GateRef glue)
{
    Label success(&builder_);
    Label callRuntime(&builder_);
    Label exit(&builder_);
    auto initialHClass = acc_.GetValueIn(gate, 0);
    auto size = builder_.GetObjectSizeFromHClass(initialHClass);
    auto topOffset = JSThread::GlueData::GetNewSpaceAllocationTopAddressOffset(false);
    auto endOffset = JSThread::GlueData::GetNewSpaceAllocationEndAddressOffset(false);
    auto topAddress = builder_.Load(VariableType::NATIVE_POINTER(), glue, builder_.IntPtr(topOffset));
    auto endAddress = builder_.Load(VariableType::NATIVE_POINTER(), glue, builder_.IntPtr(endOffset));
    auto top = builder_.Load(VariableType::JS_POINTER(), topAddress, builder_.IntPtr(0));
    auto end = builder_.Load(VariableType::JS_POINTER(), endAddress, builder_.IntPtr(0));
    DEFVAlUE(result, (&builder_), VariableType::JS_ANY(), builder_.HoleConstant());
    auto newTop = builder_.PtrAdd(top, size);
    builder_.Branch(builder_.IntPtrGreaterThan(newTop, end), &callRuntime, &success);
    builder_.Bind(&success);
    {
        builder_.Store(VariableType::NATIVE_POINTER(), glue, topAddress, builder_.IntPtr(0), newTop);
        result = top;
        builder_.Jump(&exit);
    }
    builder_.Bind(&callRuntime);
    {
        result = LowerCallRuntime(glue, gate, RTSTUB_ID(AllocateInYoung),  {builder_.ToTaggedInt(size)}, true);
        builder_.Jump(&exit);
    }
    builder_.Bind(&exit);

    // initialization
    Label afterInitialize(&builder_);
    InitializeWithSpeicalValue(&afterInitialize, *result, glue, builder_.HoleConstant(),
                               builder_.Int32(JSObject::SIZE), builder_.TruncInt64ToInt32(size));
    builder_.Bind(&afterInitialize);
    builder_.Store(VariableType::JS_POINTER(), glue, *result, builder_.IntPtr(0), initialHClass);
    GateRef hashOffset = builder_.IntPtr(ECMAObject::HASH_OFFSET);
    builder_.Store(VariableType::INT64(), glue, *result, hashOffset, builder_.Int64(JSTaggedValue(0).GetRawData()));

    GateRef emptyArray = builder_.GetGlobalConstantValue(ConstantIndex::EMPTY_ARRAY_OBJECT_INDEX);
    GateRef propertiesOffset = builder_.IntPtr(JSObject::PROPERTIES_OFFSET);
    builder_.Store(VariableType::JS_POINTER(), glue, *result, propertiesOffset, emptyArray);
    GateRef elementsOffset = builder_.IntPtr(JSObject::ELEMENTS_OFFSET);
    builder_.Store(VariableType::JS_POINTER(), glue, *result, elementsOffset, emptyArray);

    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), *result);
}

void TypeMCRLowering::InitializeWithSpeicalValue(Label *exit, GateRef object, GateRef glue,
                                                 GateRef value, GateRef start, GateRef end)
{
    Label begin(&builder_);
    Label storeValue(&builder_);
    Label endLoop(&builder_);

    DEFVAlUE(startOffset, (&builder_), VariableType::INT32(), start);
    builder_.Jump(&begin);
    builder_.LoopBegin(&begin);
    {
        builder_.Branch(builder_.Int32UnsignedLessThan(*startOffset, end), &storeValue, exit);
        builder_.Bind(&storeValue);
        {
            builder_.Store(VariableType::INT64(), glue, object, builder_.ZExtInt32ToPtr(*startOffset), value);
            startOffset = builder_.Int32Add(*startOffset, builder_.Int32(JSTaggedValue::TaggedTypeSize()));
            builder_.Jump(&endLoop);
        }
        builder_.Bind(&endLoop);
        builder_.LoopEnd(&begin);
    }
}

void TypeMCRLowering::LowerTypedBinaryOp(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    TypedBinOp op = acc_.GetTypedBinaryOp(gate);
    switch (op) {
        case TypedBinOp::TYPED_MOD: {
            LowerTypedMod(gate);
            break;
        }
        default:
            LOG_COMPILER(FATAL) << "unkown TypedBinOp: " << static_cast<int>(op);
            break;
    }
}

void TypeMCRLowering::LowerTypedMod(GateRef gate)
{
    auto leftType = acc_.GetLeftType(gate);
    auto rightType = acc_.GetRightType(gate);
    if (leftType.IsNumberType() && rightType.IsNumberType()) {
        LowerNumberMod(gate);
    } else {
        LOG_ECMA(FATAL) << "this branch is unreachable";
        UNREACHABLE();
    }
}

void TypeMCRLowering::LowerNumberMod(GateRef gate)
{
    GateRef left = acc_.GetValueIn(gate, 0);
    GateRef right = acc_.GetValueIn(gate, 1);
    GateType leftType = acc_.GetLeftType(gate);
    GateType rightType = acc_.GetRightType(gate);
    DEFVAlUE(result, (&builder_), VariableType::JS_ANY(), builder_.HoleConstant());
    result = ModNumbers(left, right, leftType, rightType);
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), *result);
}

GateRef TypeMCRLowering::DoubleToTaggedDoublePtr(GateRef gate)
{
    return builder_.DoubleToTaggedDoublePtr(gate);
}

GateRef TypeMCRLowering::ChangeInt32ToFloat64(GateRef gate)
{
    return builder_.ChangeInt32ToFloat64(gate);
}

GateRef TypeMCRLowering::TruncDoubleToInt(GateRef gate)
{
    return builder_.TruncInt64ToInt32(builder_.TruncFloatToInt64(gate));
}

GateRef TypeMCRLowering::Int32Mod(GateRef left, GateRef right)
{
    return builder_.BinaryArithmetic(circuit_->Smod(), MachineType::I32, left, right);
}

GateRef TypeMCRLowering::DoubleMod(GateRef left, GateRef right)
{
    return builder_.BinaryArithmetic(circuit_->Fmod(), MachineType::F64, left, right);
}

GateRef TypeMCRLowering::IntToTaggedIntPtr(GateRef x)
{
    GateRef val = builder_.SExtInt32ToInt64(x);
    return builder_.ToTaggedIntPtr(val);
}

GateRef TypeMCRLowering::ModNumbers(GateRef left, GateRef right, GateType leftType, GateType rightType)
{
    auto env = builder_.GetCurrentEnvironment();
    Label entry(&builder_);
    env->SubCfgEntry(&entry);
    DEFVAlUE(result, (&builder_), VariableType::JS_ANY(), builder_.HoleConstant());
    DEFVAlUE(intLeft, (&builder_), VariableType::INT32(), builder_.Int32(0));
    DEFVAlUE(intRight, (&builder_), VariableType::INT32(), builder_.Int32(0));
    DEFVAlUE(doubleLeft, (&builder_), VariableType::FLOAT64(), builder_.Double(0));
    DEFVAlUE(doubleRight, (&builder_), VariableType::FLOAT64(), builder_.Double(0));

    Label exit(&builder_);
    Label doFloatOp(&builder_);
    Label doIntOp(&builder_);
    Label leftIsIntRightIsDouble(&builder_);
    Label rightIsInt(&builder_);
    Label rightIsDouble(&builder_);
    Label leftIsInt(&builder_);
    Label leftIsDouble(&builder_);
    bool intOptAccessed = false;

    auto LowerIntOpInt = [&]() -> void {
        builder_.Jump(&doIntOp);
        intOptAccessed = true;
    };
    auto LowerDoubleOpInt = [&]() -> void {
        doubleLeft = builder_.GetDoubleOfTDouble(left);
        doubleRight = ChangeInt32ToFloat64(builder_.GetInt32OfTInt(right));
        builder_.Jump(&doFloatOp);
    };
    auto LowerIntOpDouble = [&]() -> void {
        doubleLeft = ChangeInt32ToFloat64(builder_.GetInt32OfTInt(left));
        doubleRight = builder_.GetDoubleOfTDouble(right);
        builder_.Jump(&doFloatOp);
    };
    auto LowerDoubleOpDouble = [&]() -> void {
        doubleLeft = builder_.GetDoubleOfTDouble(left);
        doubleRight = builder_.GetDoubleOfTDouble(right);
        builder_.Jump(&doFloatOp);
    };
    auto LowerRightWhenLeftIsInt = [&]() -> void {
        if (rightType.IsIntType()) {
            LowerIntOpInt();
        } else if (rightType.IsDoubleType()) {
            LowerIntOpDouble();
        } else {
            builder_.Branch(builder_.TaggedIsInt(right), &doIntOp, &leftIsIntRightIsDouble);
            intOptAccessed = true;
            builder_.Bind(&leftIsIntRightIsDouble);
            LowerIntOpDouble();
        }
    };
    auto LowerRightWhenLeftIsDouble = [&]() -> void {
        if (rightType.IsIntType()) {
            LowerDoubleOpInt();
        } else if (rightType.IsDoubleType()) {
            LowerDoubleOpDouble();
        } else {
            builder_.Branch(builder_.TaggedIsInt(right), &rightIsInt, &rightIsDouble);
            builder_.Bind(&rightIsInt);
            LowerDoubleOpInt();
            builder_.Bind(&rightIsDouble);
            LowerDoubleOpDouble();
        }
    };

    if (leftType.IsIntType()) {
        // left is int
        LowerRightWhenLeftIsInt();
    } else if (leftType.IsDoubleType()) {
        // left is double
        LowerRightWhenLeftIsDouble();
    } else {
        // left is number and need typecheck in runtime
        builder_.Branch(builder_.TaggedIsInt(left), &leftIsInt, &leftIsDouble);
        builder_.Bind(&leftIsInt);
        LowerRightWhenLeftIsInt();
        builder_.Bind(&leftIsDouble);
        LowerRightWhenLeftIsDouble();
    }
    if (intOptAccessed) {
        builder_.Bind(&doIntOp);
        {
            intLeft = builder_.GetInt32OfTInt(left);
            intRight = builder_.GetInt32OfTInt(right);
            doubleLeft = ChangeInt32ToFloat64(builder_.GetInt32OfTInt(left));
            doubleRight = ChangeInt32ToFloat64(builder_.GetInt32OfTInt(right));
            Label leftGreaterZero(&builder_);
            builder_.Branch(builder_.Int32GreaterThan(*intLeft, builder_.Int32(0)),
                            &leftGreaterZero, &doFloatOp);
            builder_.Bind(&leftGreaterZero);
            {
                Label rightGreaterZero(&builder_);
                builder_.Branch(builder_.Int32GreaterThan(*intRight, builder_.Int32(0)),
                                &rightGreaterZero, &doFloatOp);
                builder_.Bind(&rightGreaterZero);
                {
                    result = IntToTaggedIntPtr(Int32Mod(*intLeft, *intRight));
                    builder_.Jump(&exit);
                }
            }
        }
    }
    builder_.Bind(&doFloatOp);
    {
        Label rightNotZero(&builder_);
        Label rightIsZeroOrNanOrLeftIsNanOrInf(&builder_);
        Label rightNotZeroAndNanAndLeftNotNanAndInf(&builder_);
        builder_.Branch(builder_.Equal(*doubleRight, builder_.Double(0.0)), &rightIsZeroOrNanOrLeftIsNanOrInf,
                        &rightNotZero);
        builder_.Bind(&rightNotZero);
        {
            Label rightNotNan(&builder_);
            builder_.Branch(builder_.DoubleIsNAN(*doubleRight), &rightIsZeroOrNanOrLeftIsNanOrInf, &rightNotNan);
            builder_.Bind(&rightNotNan);
            {
                Label leftNotNan(&builder_);
                builder_.Branch(builder_.DoubleIsNAN(*doubleLeft), &rightIsZeroOrNanOrLeftIsNanOrInf, &leftNotNan);
                builder_.Bind(&leftNotNan);
                builder_.Branch(builder_.DoubleIsINF(*doubleLeft),
                                &rightIsZeroOrNanOrLeftIsNanOrInf,
                                &rightNotZeroAndNanAndLeftNotNanAndInf);
            }
        }
        builder_.Bind(&rightIsZeroOrNanOrLeftIsNanOrInf);
        {
            result = DoubleToTaggedDoublePtr(builder_.Double(base::NAN_VALUE));
            builder_.Jump(&exit);
        }
        builder_.Bind(&rightNotZeroAndNanAndLeftNotNanAndInf);
        {
            Label leftNotZero(&builder_);
            Label leftIsZeroOrRightIsInf(&builder_);
            builder_.Branch(builder_.Equal(*doubleLeft, builder_.Double(0.0)), &leftIsZeroOrRightIsInf,
                            &leftNotZero);
            builder_.Bind(&leftNotZero);
            {
                Label rightNotInf(&builder_);
                builder_.Branch(builder_.DoubleIsINF(*doubleRight), &leftIsZeroOrRightIsInf, &rightNotInf);
                builder_.Bind(&rightNotInf);
                {
                    GateRef glue = acc_.GetGlueFromArgList();
                    result = builder_.CallNGCRuntime(
                        glue, RTSTUB_ID(FloatMod), Gate::InvalidGateRef, {*doubleLeft, *doubleRight},
                        Circuit::NullGate());
                    builder_.Jump(&exit);
                }
            }
            builder_.Bind(&leftIsZeroOrRightIsInf);
            {
                result = DoubleToTaggedDoublePtr(*doubleLeft);
                builder_.Jump(&exit);
            }
        }
    }
    builder_.Bind(&exit);
    auto ret = *result;
    env->SubCfgExit();
    return ret;
}

void TypeMCRLowering::LowerTypedCallBuitin(GateRef gate)
{
    BuiltinLowering lowering(circuit_);
    lowering.LowerTypedCallBuitin(gate);
}

void TypeMCRLowering::LowerJSCallTargetTypeCheck(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    auto type = acc_.GetParamGateType(gate);
    if (tsManager_->IsFunctionTypeKind(type)) {
        ArgumentAccessor argAcc(circuit_);
        GateRef frameState = GetFrameState(gate);
        GateRef jsFunc = argAcc.GetFrameArgsIn(frameState, FrameArgIdx::FUNC);
        auto func = acc_.GetValueIn(gate, 0);
        auto methodIndex = acc_.GetValueIn(gate, 1);
        GateRef isObj = builder_.TaggedIsHeapObject(func);
        GateRef isJsFunc = builder_.IsJSFunction(func);
        GateRef funcMethodTarget = builder_.GetMethodFromFunction(func);
        GateRef isAot = builder_.HasAotCode(funcMethodTarget);
        GateRef checkFunc = builder_.BoolAnd(isObj, isJsFunc);
        GateRef checkAot = builder_.BoolAnd(checkFunc, isAot);
        GateRef methodTarget = GetObjectFromConstPool(jsFunc, methodIndex);
        GateRef check = builder_.BoolAnd(checkAot, builder_.Equal(funcMethodTarget, methodTarget));
        builder_.DeoptCheck(check, frameState, DeoptType::NOTJSCALLTGT);
        acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), Circuit::NullGate());
    } else {
        LOG_ECMA(FATAL) << "this branch is unreachable";
        UNREACHABLE();
    }
}

void TypeMCRLowering::LowerJSFastCallTargetTypeCheck(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    auto type = acc_.GetParamGateType(gate);
    if (tsManager_->IsFunctionTypeKind(type)) {
        ArgumentAccessor argAcc(circuit_);
        GateRef frameState = GetFrameState(gate);
        GateRef jsFunc = argAcc.GetFrameArgsIn(frameState, FrameArgIdx::FUNC);
        auto func = acc_.GetValueIn(gate, 0);
        auto methodIndex = acc_.GetValueIn(gate, 1);
        GateRef isObj = builder_.TaggedIsHeapObject(func);
        GateRef isJsFunc = builder_.IsJSFunction(func);
        GateRef funcMethodTarget = builder_.GetMethodFromFunction(func);
        GateRef canFastCall = builder_.HasAotCodeAndFastCall(funcMethodTarget);
        GateRef checkFunc = builder_.BoolAnd(isObj, isJsFunc);
        GateRef checkAot = builder_.BoolAnd(checkFunc, canFastCall);
        GateRef methodTarget = GetObjectFromConstPool(jsFunc, methodIndex);
        GateRef check = builder_.BoolAnd(checkAot, builder_.Equal(funcMethodTarget, methodTarget));
        builder_.DeoptCheck(check, frameState, DeoptType::NOTJSFASTCALLTGT);
        acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), Circuit::NullGate());
    } else {
        LOG_ECMA(FATAL) << "this branch is unreachable";
        UNREACHABLE();
    }
}

void TypeMCRLowering::LowerJSCallThisTargetTypeCheck(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    auto type = acc_.GetParamGateType(gate);
    if (tsManager_->IsFunctionTypeKind(type)) {
        GateRef frameState = GetFrameState(gate);
        auto func = acc_.GetValueIn(gate, 0);
        GateRef isObj = builder_.TaggedIsHeapObject(func);
        GateRef isJsFunc = builder_.IsJSFunction(func);
        GateRef checkFunc = builder_.BoolAnd(isObj, isJsFunc);
        GateRef funcMethodTarget = builder_.GetMethodFromFunction(func);
        GateRef isAot = builder_.HasAotCode(funcMethodTarget);
        GateRef check = builder_.BoolAnd(checkFunc, isAot);
        builder_.DeoptCheck(check, frameState, DeoptType::NOTJSCALLTGT);
        acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), Circuit::NullGate());
    } else {
        LOG_ECMA(FATAL) << "this branch is unreachable";
        UNREACHABLE();
    }
}

void TypeMCRLowering::LowerJSFastCallThisTargetTypeCheck(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    auto type = acc_.GetParamGateType(gate);
    if (tsManager_->IsFunctionTypeKind(type)) {
        GateRef frameState = GetFrameState(gate);
        auto func = acc_.GetValueIn(gate, 0);
        GateRef isObj = builder_.TaggedIsHeapObject(func);
        GateRef isJsFunc = builder_.IsJSFunction(func);
        GateRef checkFunc = builder_.BoolAnd(isObj, isJsFunc);
        GateRef funcMethodTarget = builder_.GetMethodFromFunction(func);
        GateRef canFastCall = builder_.HasAotCodeAndFastCall(funcMethodTarget);
        GateRef check = builder_.BoolAnd(checkFunc, canFastCall);
        builder_.DeoptCheck(check, frameState, DeoptType::NOTJSFASTCALLTGT);
        acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), Circuit::NullGate());
    } else {
        LOG_ECMA(FATAL) << "this branch is unreachable";
        UNREACHABLE();
    }
}

void TypeMCRLowering::LowerCallTargetCheck(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    GateRef frameState = GetFrameState(gate);

    BuiltinLowering lowering(circuit_);
    GateRef funcheck = lowering.LowerCallTargetCheck(&env, gate);
    GateRef check = lowering.CheckPara(gate, funcheck);
    builder_.DeoptCheck(check, frameState, DeoptType::NOTCALLTGT);

    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), Circuit::NullGate());
}

void TypeMCRLowering::LowerJSInlineTargetTypeCheck(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    GateRef frameState = GetFrameState(gate);
    auto func = acc_.GetValueIn(gate, 0);
    GateRef isObj = builder_.TaggedIsHeapObject(func);
    GateRef isJsFunc = builder_.IsJSFunction(func);
    GateRef checkFunc = builder_.BoolAnd(isObj, isJsFunc);
    GateRef GetMethodId = builder_.GetMethodId(func);
    GateRef check = builder_.BoolAnd(checkFunc, builder_.Equal(GetMethodId, acc_.GetValueIn(gate, 1)));
    builder_.DeoptCheck(check, frameState, DeoptType::INLINEFAIL);
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), Circuit::NullGate());
}

void TypeMCRLowering::LowerTypedNewAllocateThis(GateRef gate, GateRef glue)
{
    Environment env(gate, circuit_, &builder_);
    ArgumentAccessor argAcc(circuit_);
    GateRef frameState = GetFrameState(gate);
    GateRef jsFunc = argAcc.GetFrameArgsIn(frameState, FrameArgIdx::FUNC);

    GateRef ctor = acc_.GetValueIn(gate, 0);

    DEFVAlUE(thisObj, (&builder_), VariableType::JS_ANY(), builder_.Undefined());
    Label allocate(&builder_);
    Label exit(&builder_);

    GateRef isBase = builder_.IsBase(ctor);
    builder_.Branch(isBase, &allocate, &exit);
    builder_.Bind(&allocate);
    {
        // add typecheck to detect protoOrHclass is equal with ihclass,
        // if pass typecheck: 1.no need to check whether hclass is valid 2.no need to check return result
        GateRef protoOrHclass = builder_.Load(VariableType::JS_ANY(), ctor,
                                              builder_.IntPtr(JSFunction::PROTO_OR_DYNCLASS_OFFSET));
        GateRef ihclassIndex = acc_.GetValueIn(gate, 1);
        GateRef ihclass = GetObjectFromConstPool(jsFunc, ihclassIndex);
        GateRef check = builder_.Equal(protoOrHclass, ihclass);
        builder_.DeoptCheck(check, frameState, DeoptType::NOTNEWOBJ);

        thisObj = builder_.CallStub(glue, gate, CommonStubCSigns::NewJSObject, { glue, protoOrHclass });
        builder_.Jump(&exit);
    }
    builder_.Bind(&exit);
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), *thisObj);
}

void TypeMCRLowering::LowerTypedSuperAllocateThis(GateRef gate, GateRef glue)
{
    Environment env(gate, circuit_, &builder_);
    GateRef superCtor = acc_.GetValueIn(gate, 0);
    GateRef newTarget = acc_.GetValueIn(gate, 1);

    DEFVAlUE(thisObj, (&builder_), VariableType::JS_ANY(), builder_.Undefined());
    Label allocate(&builder_);
    Label exit(&builder_);

    GateRef isBase = builder_.IsBase(superCtor);
    builder_.Branch(isBase, &allocate, &exit);
    builder_.Bind(&allocate);
    {
        GateRef protoOrHclass = builder_.Load(VariableType::JS_ANY(), newTarget,
                                              builder_.IntPtr(JSFunction::PROTO_OR_DYNCLASS_OFFSET));
        GateRef check = builder_.IsJSHClass(protoOrHclass);
        GateRef frameState = GetFrameState(gate);
        builder_.DeoptCheck(check, frameState, DeoptType::NOTNEWOBJ);

        thisObj = builder_.CallStub(glue, gate, CommonStubCSigns::NewJSObject, { glue, protoOrHclass });
        builder_.Jump(&exit);
    }
    builder_.Bind(&exit);
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), *thisObj);
}

void TypeMCRLowering::LowerGetSuperConstructor(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    GateRef ctor = acc_.GetValueIn(gate, 0);
    GateRef hclass = builder_.LoadHClass(ctor);
    GateRef protoOffset = builder_.IntPtr(JSHClass::PROTOTYPE_OFFSET);
    GateRef superCtor = builder_.Load(VariableType::JS_ANY(), hclass, protoOffset);
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), superCtor);
}

GateRef TypeMCRLowering::LoadFromTaggedArray(GateRef array, size_t index)
{
    auto dataOffset = TaggedArray::DATA_OFFSET + index * JSTaggedValue::TaggedTypeSize();
    return builder_.LoadConstOffset(VariableType::JS_ANY(), array, dataOffset);
}

GateRef TypeMCRLowering::LoadFromVTable(GateRef receiver, size_t index)
{
    GateRef hclass = builder_.LoadConstOffset(
        VariableType::JS_POINTER(), receiver, TaggedObject::HCLASS_OFFSET);
    GateRef vtable = builder_.LoadConstOffset(VariableType::JS_ANY(),
        hclass, JSHClass::VTABLE_OFFSET);

    GateRef itemOwner = LoadFromTaggedArray(vtable, VTable::TupleItem::OWNER + index);
    GateRef itemOffset = LoadFromTaggedArray(vtable, VTable::TupleItem::OFFSET + index);
    return builder_.Load(VariableType::JS_ANY(), itemOwner, builder_.TaggedGetInt(itemOffset));
}

GateRef TypeMCRLowering::LoadSupers(GateRef hclass)
{
    return builder_.LoadConstOffset(VariableType::JS_ANY(), hclass, JSHClass::SUPERS_OFFSET);
}

GateRef TypeMCRLowering::GetLengthFromSupers(GateRef supers)
{
    return builder_.LoadConstOffset(VariableType::INT32(), supers, TaggedArray::EXTRACT_LENGTH_OFFSET);
}

GateRef TypeMCRLowering::GetValueFromSupers(GateRef supers, size_t index)
{
    GateRef val = LoadFromTaggedArray(supers, index);
    return builder_.LoadObjectFromWeakRef(val);
}

GateRef TypeMCRLowering::CallAccessor(GateRef glue, GateRef gate, GateRef function, GateRef receiver, AccessorMode mode,
                                      GateRef value)
{
    const CallSignature *cs = RuntimeStubCSigns::Get(RTSTUB_ID(JSCall));
    GateRef target = builder_.IntPtr(RTSTUB_ID(JSCall));
    GateRef newTarget = builder_.Undefined();
    GateRef argc = builder_.Int64(NUM_MANDATORY_JSFUNC_ARGS + (mode == AccessorMode::SETTER ? 1 : 0));  // 1: value
    std::vector<GateRef> args { glue, argc, function, newTarget, receiver };
    if (mode == AccessorMode::SETTER) {
        args.emplace_back(value);
    }

    return builder_.Call(cs, glue, target, builder_.GetDepend(), args, gate);
}

void TypeMCRLowering::ReplaceHirWithPendingException(GateRef hirGate, GateRef glue, GateRef state, GateRef depend,
                                                     GateRef value)
{
    auto condition = builder_.HasPendingException(glue);
    GateRef ifBranch = builder_.Branch(state, condition);
    GateRef ifTrue = builder_.IfTrue(ifBranch);
    GateRef ifFalse = builder_.IfFalse(ifBranch);
    GateRef eDepend = builder_.DependRelay(ifTrue, depend);
    GateRef sDepend = builder_.DependRelay(ifFalse, depend);

    StateDepend success(ifFalse, sDepend);
    StateDepend exception(ifTrue, eDepend);
    acc_.ReplaceHirWithIfBranch(hirGate, success, exception, value);
}
}  // namespace panda::ecmascript