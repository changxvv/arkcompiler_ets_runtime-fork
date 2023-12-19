/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "jsvaluerefislightweightmapandset_fuzzer.h"
#include "ecmascript/containers/containers_list.h"
#include "ecmascript/containers/containers_private.h"
#include "ecmascript/ecma_string-inl.h"
#include "ecmascript/ecma_vm.h"
#include "ecmascript/global_env.h"
#include "ecmascript/js_handle.h"
#include "ecmascript/js_tagged_value.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/js_global_object.h"
#include "ecmascript/napi/jsnapi_helper.h"
#include "ecmascript/linked_hash_table.h"
#include "ecmascript/ecma_runtime_call_info.h"
#include "ecmascript/containers/containers_hashset.h"
#include "ecmascript/common.h"
#include "ecmascript/frames.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/js_set.h"
#include "ecmascript/js_set_iterator.h"
#include "ecmascript/js_map.h"
#include "ecmascript/js_weak_container.h"
#include "ecmascript/js_map_iterator.h"
#include "ecmascript/containers/containers_arraylist.h"
#include "ecmascript/js_api/js_api_arraylist.h"
#include "ecmascript/builtins/builtins_function.h"
#include "ecmascript/builtins/builtins.h"
#include "ecmascript/ecma_global_storage.h"
#include "ecmascript/js_bigint.h"
#include "ecmascript/js_runtime_options.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/tagged_array.h"
#include "ecmascript/js_generator_object.h"
#include "ecmascript/js_string_iterator.h"
#include "ecmascript/js_date_time_format.h"
#include "ecmascript/js_tagged_number.h"
#include "ecmascript/js_api/js_api_hashmap.h"
#include "ecmascript/builtins/builtins_regexp.h"
#include "ecmascript/js_regexp.h"
#include "ecmascript/tagged_hash_array.h"
#include "ecmascript/containers/containers_lightweightmap.h"
#include "ecmascript/containers/containers_lightweightset.h"

using namespace panda;
using namespace panda::test;
using namespace panda::ecmascript;
using namespace panda::ecmascript::containers;

namespace OHOS {
EcmaRuntimeCallInfo *CreateEcmaRuntimeCallInfo(JSThread *thread, JSTaggedValue newTgt, uint32_t argvLength)
{
    const uint8_t testDecodedSize = 2;
    int32_t numActualArgs = argvLength / testDecodedSize + 1;
    JSTaggedType *sp = const_cast<JSTaggedType *>(thread->GetCurrentSPFrame());

    size_t frameSize = 0;
    if (thread->IsAsmInterpreter()) {
        frameSize = InterpretedEntryFrame::NumOfMembers() + numActualArgs;
    } else {
        frameSize = InterpretedFrame::NumOfMembers() + numActualArgs;
    }
    JSTaggedType *newSp = sp - frameSize; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (int i = numActualArgs; i > 0; i--) {
        newSp[i - 1] = JSTaggedValue::Undefined().GetRawData();
    }
    EcmaRuntimeCallInfo *ecmaRuntimeCallInfo = reinterpret_cast<EcmaRuntimeCallInfo *>(newSp - 2);
    *(--newSp) = numActualArgs;
    *(--newSp) = panda::ecmascript::ToUintPtr(thread);
    ecmaRuntimeCallInfo->SetNewTarget(newTgt);
    return ecmaRuntimeCallInfo;
}

static JSTaggedType *SetupFrame(JSThread *thread, EcmaRuntimeCallInfo *info)
{
    JSTaggedType *sp = const_cast<JSTaggedType *>(thread->GetCurrentSPFrame());
    size_t frameSize = 0;
    if (thread->IsAsmInterpreter()) {
        // 2 means thread and numArgs
        frameSize = InterpretedEntryFrame::NumOfMembers() + info->GetArgsNumber() + NUM_MANDATORY_JSFUNC_ARGS + 2;
    } else {
        // 2 means thread and numArgs
        frameSize = InterpretedFrame::NumOfMembers() + info->GetArgsNumber() + NUM_MANDATORY_JSFUNC_ARGS + 2;
    }
    JSTaggedType *newSp = sp - frameSize; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    InterpretedEntryFrame *state = reinterpret_cast<InterpretedEntryFrame *>(newSp) - 1;
    state->base.type = ecmascript::FrameType::INTERPRETER_ENTRY_FRAME;
    state->base.prev = sp;
    state->pc = nullptr;
    thread->SetCurrentSPFrame(newSp);
    return sp;
}

void TearDownFrame(JSThread *thread, JSTaggedType *prev)
{
    thread->SetCurrentSPFrame(prev);
}

JSHandle<JSAPILightWeightMap> ConstructobjectLightWeightMap(JSThread *thread)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSTaggedValue> globalObject = env->GetJSGlobalObject();
    JSHandle<JSTaggedValue> key(factory->NewFromASCII("ArkPrivate"));
    JSHandle<JSTaggedValue> value =
        JSObject::GetProperty(thread, JSHandle<JSTaggedValue>(globalObject), key).GetValue();
    auto objCallInfo1 = CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    objCallInfo1->SetFunction(JSTaggedValue::Undefined());
    objCallInfo1->SetThis(value.GetTaggedValue());
    objCallInfo1->SetCallArg(0, JSTaggedValue(static_cast<int>(containers::ContainerTag::LightWeightMap)));
    [[maybe_unused]] auto prev1 = SetupFrame(thread, objCallInfo1);
    JSTaggedValue result1 = ecmascript::containers::ContainersPrivate::Load(objCallInfo1);
    JSHandle<JSFunction> newTarget(thread, result1);
    auto objCallInfo = CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    objCallInfo->SetFunction(newTarget.GetTaggedValue());
    objCallInfo->SetNewTarget(newTarget.GetTaggedValue());
    objCallInfo->SetThis(JSTaggedValue::Undefined());
    [[maybe_unused]] auto prev = SetupFrame(thread, objCallInfo);
    JSTaggedValue result = ecmascript::containers::ContainersLightWeightMap::LightWeightMapConstructor(objCallInfo);
    TearDownFrame(thread, prev);
    JSHandle<JSAPILightWeightMap> mapHandle(thread, result);
    return mapHandle;
}

JSHandle<JSAPILightWeightSet> ConstructobjectLightWeightSet(JSThread *thread)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSTaggedValue> globalObject = env->GetJSGlobalObject();
    JSHandle<JSTaggedValue> key(factory->NewFromASCII("ArkPrivate"));
    JSHandle<JSTaggedValue> value =
        JSObject::GetProperty(thread, JSHandle<JSTaggedValue>(globalObject), key).GetValue();
    auto objCallInfo1 = CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    objCallInfo1->SetFunction(JSTaggedValue::Undefined());
    objCallInfo1->SetThis(value.GetTaggedValue());
    objCallInfo1->SetCallArg(0, JSTaggedValue(static_cast<int>(containers::ContainerTag::LightWeightSet)));
    [[maybe_unused]] auto prev1 = SetupFrame(thread, objCallInfo1);
    JSTaggedValue result1 = ecmascript::containers::ContainersPrivate::Load(objCallInfo1);
    JSHandle<JSFunction> newTarget(thread, result1);
    auto objCallInfo = CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    objCallInfo->SetFunction(newTarget.GetTaggedValue());
    objCallInfo->SetNewTarget(newTarget.GetTaggedValue());
    objCallInfo->SetThis(JSTaggedValue::Undefined());
    [[maybe_unused]] auto prev = SetupFrame(thread, objCallInfo);
    JSTaggedValue result = ecmascript::containers::ContainersLightWeightSet::LightWeightSetConstructor(objCallInfo);
    TearDownFrame(thread, prev);
    JSHandle<JSAPILightWeightSet> mapHandle(thread, result);
    return mapHandle;
}

void JSValueRefIsLightWeightMapFuzzTest([[maybe_unused]] const uint8_t *data, [[maybe_unused]] size_t size)
{
    RuntimeOption option;
    option.SetLogLevel(RuntimeOption::LOG_LEVEL::ERROR);
    EcmaVM *vm = JSNApi::CreateJSVM(option);
    auto thread = vm->GetAssociatedJSThread();
    JSHandle<JSAPILightWeightMap> mapHandle = ConstructobjectLightWeightMap(thread);
    JSHandle<JSTaggedValue> jshashmap = JSHandle<JSTaggedValue>::Cast(mapHandle);
    Local<JSValueRef> tag = JSNApiHelper::ToLocal<JSValueRef>(jshashmap);
    tag->IsLightWeightMap();
    JSNApi::DestroyJSVM(vm);
}

void JSValueRefIsLightWeightSetFuzzTest([[maybe_unused]] const uint8_t *data, [[maybe_unused]] size_t size)
{
    RuntimeOption option;
    option.SetLogLevel(RuntimeOption::LOG_LEVEL::ERROR);
    EcmaVM *vm = JSNApi::CreateJSVM(option);
    auto thread = vm->GetAssociatedJSThread();
    JSHandle<JSAPILightWeightSet> mapHandle = ConstructobjectLightWeightSet(thread);
    JSHandle<JSTaggedValue> jshashmap = JSHandle<JSTaggedValue>::Cast(mapHandle);
    Local<JSValueRef> tag = JSNApiHelper::ToLocal<JSValueRef>(jshashmap);
    tag->IsLightWeightSet();
    JSNApi::DestroyJSVM(vm);
}
}

// Fuzzer entry point.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    // Run your code on data.
    OHOS::JSValueRefIsLightWeightMapFuzzTest(data, size);
    OHOS::JSValueRefIsLightWeightSetFuzzTest(data, size);
    return 0;
}