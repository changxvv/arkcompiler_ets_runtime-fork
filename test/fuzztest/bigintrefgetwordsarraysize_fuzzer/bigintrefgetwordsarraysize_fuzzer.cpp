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

#include "bigintrefgetwordsarraysize_fuzzer.h"
#include "ecmascript/ecma_string-inl.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "ecmascript/log_wrapper.h"

using namespace panda;
using namespace panda::ecmascript;
#define MAXBYTELEN sizeof(uint64_t)
namespace OHOS {
void BigIntRefGetWordsArraySize(const uint8_t *data, size_t size)
{
    RuntimeOption option;
    option.SetLogLevel(RuntimeOption::LOG_LEVEL::ERROR);
    EcmaVM *vm = JSNApi::CreateJSVM(option);
    if (data == nullptr || size <= 0) {
        LOG_ECMA(ERROR) << "illegal input!";
        return;
    }
    if (size > MAXBYTELEN) {
        size = MAXBYTELEN;
    }
    bool sign = false;
    uint64_t words[1] = {0};
    if (memcpy_s(words, MAXBYTELEN, data, size) != 0) {
        LOG_ECMA(ERROR) << "memcpy_s failed!";
    }
    Local<JSValueRef> bigWords = BigIntRef::CreateBigWords(vm, sign, (uint32_t)size, words);
    Local<BigIntRef> bigWordsRef(bigWords);
    bigWordsRef->GetWordsArraySize();
    JSNApi::DestroyJSVM(vm);
}
}

// Fuzzer entry point.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    // Run your code on data.
    OHOS::BigIntRefGetWordsArraySize(data, size);
    return 0;
}