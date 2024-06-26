/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "ecmascript/base/json_parser.h"
#include "ecmascript/base/json_helper.h"

namespace panda::ecmascript::base {
JSHandle<JSTaggedValue> Internalize::InternalizeJsonProperty(JSThread *thread, const JSHandle<JSObject> &holder,
                                                             const JSHandle<JSTaggedValue> &name,
                                                             const JSHandle<JSTaggedValue> &receiver,
                                                             TransformType transformType)
{
    JSHandle<JSTaggedValue> objHandle(holder);
    JSHandle<JSTaggedValue> val = JSTaggedValue::GetProperty(thread, objHandle, name).GetValue();
    RETURN_HANDLE_IF_ABRUPT_COMPLETION(JSTaggedValue, thread);
    JSHandle<JSTaggedValue> lengthKey = thread->GlobalConstants()->GetHandledLengthString();
    if (val->IsECMAObject()) {
        JSHandle<JSObject> obj = JSTaggedValue::ToObject(thread, val);
        RETURN_HANDLE_IF_ABRUPT_COMPLETION(JSTaggedValue, thread);
        bool isArray = val->IsArray(thread);
        if (isArray) {
            JSHandle<JSTaggedValue> lenResult = JSTaggedValue::GetProperty(thread, val, lengthKey).GetValue();
            RETURN_HANDLE_IF_ABRUPT_COMPLETION(JSTaggedValue, thread);
            JSTaggedNumber lenNumber = JSTaggedValue::ToLength(thread, lenResult);
            RETURN_HANDLE_IF_ABRUPT_COMPLETION(JSTaggedValue, thread);
            uint32_t length = lenNumber.ToUint32();
            JSMutableHandle<JSTaggedValue> keyUnknow(thread, JSTaggedValue::Undefined());
            JSMutableHandle<JSTaggedValue> keyName(thread, JSTaggedValue::Undefined());
            for (uint32_t i = 0; i < length; i++) {
                // Let prop be ! ToString((I)).
                keyUnknow.Update(JSTaggedValue(i));
                keyName.Update(JSTaggedValue::ToString(thread, keyUnknow).GetTaggedValue());
                RETURN_HANDLE_IF_ABRUPT_COMPLETION(JSTaggedValue, thread);
                RecurseAndApply(thread, obj, keyName, receiver, transformType);
            }
        } else {
            // Let keys be ? EnumerableOwnPropertyNames(val, key).
            JSHandle<TaggedArray> ownerNames(JSObject::EnumerableOwnNames(thread, obj));
            RETURN_HANDLE_IF_ABRUPT_COMPLETION(JSTaggedValue, thread);
            uint32_t namesLength = ownerNames->GetLength();
            JSMutableHandle<JSTaggedValue> keyName(thread, JSTaggedValue::Undefined());
            for (uint32_t i = 0; i < namesLength; i++) {
                keyName.Update(ownerNames->Get(i));
                RecurseAndApply(thread, obj, keyName, receiver, transformType);
            }
        }
    }

    // Return ? Call(receiver, holder, « name, val »).
    const uint32_t argsLength = 2;  // 2: « name, val »
    JSHandle<JSTaggedValue> undefined = thread->GlobalConstants()->GetHandledUndefined();
    EcmaRuntimeCallInfo *info = EcmaInterpreter::NewRuntimeCallInfo(thread, receiver, objHandle, undefined, argsLength);
    RETURN_HANDLE_IF_ABRUPT_COMPLETION(JSTaggedValue, thread);
    info->SetCallArg(name.GetTaggedValue(), val.GetTaggedValue());
    JSTaggedValue result = JSFunction::Call(info);
    RETURN_HANDLE_IF_ABRUPT_COMPLETION(JSTaggedValue, thread);
    return JSHandle<JSTaggedValue>(thread, result);
}

bool Internalize::RecurseAndApply(JSThread *thread, const JSHandle<JSObject> &holder,
                                  const JSHandle<JSTaggedValue> &name, const JSHandle<JSTaggedValue> &receiver,
                                  TransformType transformType)
{
    STACK_LIMIT_CHECK(thread, false);
    JSHandle<JSTaggedValue> value = InternalizeJsonProperty(thread, holder, name, receiver, transformType);
    bool changeResult = false;

    // If newElement is undefined, then Perform ? val.[[Delete]](P).
    if (value->IsUndefined()) {
        SCheckMode sCheckMode = transformType == TransformType::SENDABLE ? SCheckMode::SKIP : SCheckMode::CHECK;
        changeResult = JSObject::DeleteProperty(thread, holder, name, sCheckMode);
    } else {
        // Perform ? CreateDataProperty(val, P, newElement)
        changeResult = JSObject::CreateDataProperty(thread, holder, name, value);
    }
    return changeResult;
}
}  // namespace panda::ecmascript::base
