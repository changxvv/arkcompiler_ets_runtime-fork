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

#include "ecmascript/compiler/ntype_mcr_lowering.h"
#include "ecmascript/dfx/vmstat/opt_code_profiler.h"
#include "ecmascript/compiler/new_object_stub_builder.h"

namespace panda::ecmascript::kungfu {
void NTypeMCRLowering::RunNTypeMCRLowering()
{
    std::vector<GateRef> gateList;
    circuit_->GetAllGates(gateList);

    for (const auto &gate : gateList) {
        Lower(gate);
    }

    if (IsLogEnabled()) {
        LOG_COMPILER(INFO) << "";
        LOG_COMPILER(INFO) << "\033[34m" << "=================="
                           << " after NTypeMCRlowering "
                           << "[" << GetMethodName() << "] "
                           << "==================" << "\033[0m";
        circuit_->PrintAllGatesWithBytecode();
        LOG_COMPILER(INFO) << "\033[34m" << "=========================== End =========================" << "\033[0m";
    }
}

void NTypeMCRLowering::Lower(GateRef gate)
{
    GateRef glue = acc_.GetGlueFromArgList();
    auto op = acc_.GetOpCode(gate);
    switch (op) {
        case OpCode::CREATE_ARRAY:
            LowerCreateArray(gate, glue);
            break;
        default:
            break;
    }
}

void NTypeMCRLowering::LowerCreateArray(GateRef gate, GateRef glue)
{
    Environment env(gate, circuit_, &builder_);
    if (acc_.GetArraySize(gate) == 0) {
        LowerCreateEmptyArray(gate);
    } else {
        LowerCreateArrayWithOwn(gate, glue);
    }
}

void NTypeMCRLowering::LowerCreateEmptyArray(GateRef gate)
{
    JSHandle<JSFunction> arrayFunc(tsManager_->GetEcmaVM()->GetGlobalEnv()->GetArrayFunction());
    JSTaggedValue protoOrHClass = arrayFunc->GetProtoOrHClass();
    JSHClass *arrayHC = JSHClass::Cast(protoOrHClass.GetTaggedObject());
    size_t arraySize = arrayHC->GetObjectSize();
    size_t lengthAccessorOffset = arrayHC->GetInlinedPropertiesOffset(JSArray::LENGTH_INLINE_PROPERTY_INDEX);

    GateRef obj = builder_.GetGlobalConstantValue(ConstantIndex::EMPTY_ARRAY_OBJECT_INDEX);
    GateRef globalEnv = builder_.GetGlobalEnv();
    GateRef accessor = builder_.GetGlobalConstantValue(ConstantIndex::ARRAY_LENGTH_ACCESSOR);
    GateRef hclass = builder_.GetGlobalEnvObjHClass(globalEnv, GlobalEnv::ARRAY_FUNCTION_INDEX);
    GateRef size = builder_.IntPtr(arrayHC->GetObjectSize());

    builder_.StartAllocate();
    GateRef array = builder_.HeapAlloc(size, GateType::TaggedValue(), RegionSpaceFlag::IN_YOUNG_SPACE);
    // initialization
    for (size_t offset = JSArray::SIZE; offset < arraySize; offset += JSTaggedValue::TaggedTypeSize()) {
        builder_.StoreConstOffset(VariableType::INT64(), array, offset, builder_.Undefined());
    }
    builder_.StoreConstOffset(VariableType::JS_POINTER(), array, 0, hclass);
    builder_.StoreConstOffset(VariableType::INT64(), array, ECMAObject::HASH_OFFSET,
                              builder_.Int64(JSTaggedValue(0).GetRawData()));
    builder_.StoreConstOffset(VariableType::JS_POINTER(), array, JSObject::PROPERTIES_OFFSET, obj);
    builder_.StoreConstOffset(VariableType::JS_POINTER(), array, JSObject::ELEMENTS_OFFSET, obj);
    builder_.StoreConstOffset(VariableType::JS_ANY(), array, JSArray::LENGTH_OFFSET,
                              builder_.Int32ToTaggedInt(builder_.Int32(0)));
    builder_.StoreConstOffset(VariableType::JS_POINTER(), array, lengthAccessorOffset, accessor);
    builder_.FinishAllocate();
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), array);
}

void NTypeMCRLowering::LowerCreateArrayWithOwn(GateRef gate, GateRef glue)
{
    size_t elementsLength = acc_.GetArraySize(gate);
    GateRef length = builder_.IntPtr(elementsLength);
    GateRef elements = Circuit::NullGate();
    if (elementsLength < MAX_TAGGED_ARRAY_LENGTH) {
        elements = NewTaggedArray(elementsLength);
    } else {
        elements = LowerCallRuntime(glue, gate, RTSTUB_ID(NewTaggedArray), { builder_.Int32ToTaggedInt(length) }, true);
    }

    JSHandle<JSFunction> arrayFunc(tsManager_->GetEcmaVM()->GetGlobalEnv()->GetArrayFunction());
    JSTaggedValue protoOrHClass = arrayFunc->GetProtoOrHClass();
    JSHClass *arrayHC = JSHClass::Cast(protoOrHClass.GetTaggedObject());
    size_t arraySize = arrayHC->GetObjectSize();
    size_t lengthAccessorOffset = arrayHC->GetInlinedPropertiesOffset(JSArray::LENGTH_INLINE_PROPERTY_INDEX);

    GateRef obj = builder_.GetGlobalConstantValue(ConstantIndex::EMPTY_ARRAY_OBJECT_INDEX);
    GateRef globalEnv = builder_.GetGlobalEnv();
    GateRef accessor = builder_.GetGlobalConstantValue(ConstantIndex::ARRAY_LENGTH_ACCESSOR);
    GateRef hclass = builder_.GetGlobalEnvObjHClass(globalEnv, GlobalEnv::ARRAY_FUNCTION_INDEX);
    GateRef size = builder_.IntPtr(arrayHC->GetObjectSize());

    builder_.StartAllocate();
    GateRef array = builder_.HeapAlloc(size, GateType::TaggedValue(), RegionSpaceFlag::IN_YOUNG_SPACE);
    // initialization
    for (size_t offset = JSArray::SIZE; offset < arraySize; offset += JSTaggedValue::TaggedTypeSize()) {
        builder_.StoreConstOffset(VariableType::INT64(), array, offset, builder_.Undefined());
    }
    builder_.StoreConstOffset(VariableType::JS_POINTER(), array, 0, hclass);
    builder_.StoreConstOffset(VariableType::INT64(), array, ECMAObject::HASH_OFFSET,
                              builder_.Int64(JSTaggedValue(0).GetRawData()));
    builder_.StoreConstOffset(VariableType::JS_POINTER(), array, JSObject::PROPERTIES_OFFSET, obj);
    builder_.StoreConstOffset(VariableType::JS_POINTER(), array, JSObject::ELEMENTS_OFFSET, elements);
    builder_.StoreConstOffset(VariableType::JS_ANY(), array, JSArray::LENGTH_OFFSET, builder_.Int32ToTaggedInt(length));
    builder_.StoreConstOffset(VariableType::JS_POINTER(), array, lengthAccessorOffset, accessor);
    builder_.FinishAllocate();
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), array);
}

GateRef NTypeMCRLowering::NewTaggedArray(size_t length)
{
    GateRef elementsHclass = builder_.GetGlobalConstantValue(ConstantIndex::ARRAY_CLASS_INDEX);
    GateRef elementsSize = builder_.ComputeTaggedArraySize(builder_.IntPtr(length));

    builder_.StartAllocate();
    GateRef elements = builder_.HeapAlloc(elementsSize, GateType::TaggedValue(), RegionSpaceFlag::IN_YOUNG_SPACE);
    builder_.StoreConstOffset(VariableType::JS_POINTER(), elements, 0, elementsHclass);
    builder_.StoreConstOffset(VariableType::JS_ANY(), elements, TaggedArray::LENGTH_OFFSET,
        builder_.Int32ToTaggedInt(builder_.IntPtr(length)));
    size_t endOffset = TaggedArray::DATA_OFFSET + length * JSTaggedValue::TaggedTypeSize();
    // initialization
    for (size_t offset = TaggedArray::DATA_OFFSET; offset < endOffset; offset += JSTaggedValue::TaggedTypeSize()) {
        builder_.StoreConstOffset(VariableType::INT64(), elements, offset, builder_.Hole());
    }
    builder_.FinishAllocate();

    return elements;
}

GateRef NTypeMCRLowering::LowerCallRuntime(GateRef glue, GateRef hirGate, int index, const std::vector<GateRef> &args,
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
}
