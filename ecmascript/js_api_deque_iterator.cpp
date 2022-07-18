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

#include "js_api_deque_iterator.h"

#include "ecmascript/base/builtins_base.h"
#include "ecmascript/ecma_handle_scope.h"
#include "ecmascript/ecma_runtime_call_info.h"
#include "ecmascript/global_env_constants.h"
#include "ecmascript/global_env_constants-inl.h"
#include "ecmascript/js_handle.h"
#include "ecmascript/js_iterator.h"
#include "ecmascript/js_tagged_value-inl.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/tagged_array.h"

#include "js_api_deque.h"

namespace panda::ecmascript {
using BuiltinsBase = base::BuiltinsBase;
// DequeIteratorPrototype%.next ( )
JSTaggedValue JSAPIDequeIterator::Next(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv);
    JSThread *thread = argv->GetThread();
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> input(BuiltinsBase::GetThis(argv));

    if (!input->IsJSAPIDequeIterator()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "this value is not an deque iterator", JSTaggedValue::Exception());
    }
    JSHandle<JSAPIDequeIterator> iter(input);
    JSHandle<JSTaggedValue> iteratorDeque(thread, iter->GetIteratedDeque());
    JSHandle<JSTaggedValue> undefinedHandle = thread->GlobalConstants()->GetHandledUndefined();
    if (iteratorDeque->IsUndefined()) {
        return JSIterator::CreateIterResultObject(thread, undefinedHandle, true).GetTaggedValue();
    }
    JSHandle<JSAPIDeque> deque = JSHandle<JSAPIDeque>::Cast(iteratorDeque);
    uint32_t index = iter->GetNextIndex();

    JSHandle<TaggedArray> elements(thread, deque->GetElements());
    uint32_t capacity = elements->GetLength();
    uint32_t first = deque->GetFirst();
    uint32_t last = deque->GetLast();
    if (index == last) {
        iter->SetIteratedDeque(thread, undefinedHandle);
        return JSIterator::CreateIterResultObject(thread, undefinedHandle, true).GetTaggedValue();
    }
    ASSERT(capacity != 0);
    iter->SetNextIndex((index + 1) % capacity);
    uint32_t elementIndex = (index + capacity - first) % capacity;
    JSHandle<JSTaggedValue> value = JSTaggedValue::GetProperty(thread, iteratorDeque, elementIndex).GetValue();

    return JSIterator::CreateIterResultObject(thread, value, false).GetTaggedValue();
}
} // namespace panda::ecmascript
