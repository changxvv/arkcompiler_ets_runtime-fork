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

#include "numberrefnewint32_fuzzer.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "ecmascript/tooling/debugger_service.h"
#include "ecmascript/ecma_string-inl.h"

using namespace panda;
using namespace panda::ecmascript;

#define INT32_BETY_SIZE sizeof(int32_t)

namespace OHOS {
    void NumberRefNewInt32FuzzTest(const uint8_t* data, size_t size)
    {
        RuntimeOption option;
        option.SetLogLevel(RuntimeOption::LOG_LEVEL::ERROR);
        auto vm = JSNApi::CreateJSVM(option);
        [[maybe_unused]] LocalScope scope(vm);
        int32_t input = 0;
        if (size > INT32_BETY_SIZE) {
            size = INT32_BETY_SIZE;
        }
        if (memcpy_s(&input, INT32_BETY_SIZE, data, size) != 0) {
            std::cout << "memcpy_s failed !" << std::endl;
            UNREACHABLE();
        }
        [[maybe_unused]] Local<NumberRef> number = NumberRef::New(vm, input);
        JSNApi::DestroyJSVM(vm);
    }
}

// Fuzzer entry point.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::NumberRefNewInt32FuzzTest(data, size);
    return 0;
}