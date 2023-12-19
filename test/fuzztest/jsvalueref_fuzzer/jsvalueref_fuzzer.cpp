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

#include "ecmascript/global_env.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "jsvalueref_fuzzer.h"

using namespace panda;
using namespace panda::ecmascript;

namespace OHOS {
    void JSValueRefIsNullFuzzTest(const uint8_t* data, size_t size)
    {
        RuntimeOption option;
        option.SetLogLevel(RuntimeOption::LOG_LEVEL::ERROR);
        EcmaVM *vm = JSNApi::CreateJSVM(option);
        if (data == nullptr || size <= 0) {
            LOG_ECMA(ERROR) << "illegal input!";
            return;
        }
        uint8_t* ptr = nullptr;
        ptr = const_cast<uint8_t*>(data);
        char16_t utf16[] = u"This is a char16 array";
        int size1 = sizeof(utf16);
        Local<StringRef> obj = StringRef::NewFromUtf16(vm, utf16, size1);
        obj->IsNull();
        JSNApi::DestroyJSVM(vm);
    }

    void JSValueRefIsBooleanFuzzTest(const uint8_t* data, size_t size)
    {
        RuntimeOption option;
        option.SetLogLevel(RuntimeOption::LOG_LEVEL::ERROR);
        EcmaVM *vm = JSNApi::CreateJSVM(option);
        if (data == nullptr || size <= 0) {
            LOG_ECMA(ERROR) << "illegal input!";
            return;
        }
        uint8_t* ptr = nullptr;
        ptr = const_cast<uint8_t*>(data);
        char16_t utf16[] = u"This is a char16 array";
        int size1 = sizeof(utf16);
        Local<StringRef> obj = StringRef::NewFromUtf16(vm, utf16, size1);
        obj->IsBoolean();
        JSNApi::DestroyJSVM(vm);
    }
}

// Fuzzer entry point.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::JSValueRefIsNullFuzzTest(data, size);
    OHOS::JSValueRefIsBooleanFuzzTest(data, size);
    return 0;
}