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

#ifndef ECMASCRIPT_BUILTINS_BUILTINS_METHOD_INDEX_H
#define ECMASCRIPT_BUILTINS_BUILTINS_METHOD_INDEX_H

#include <stdint.h>

namespace panda::ecmascript {
enum class MethodIndex : uint8_t {
    BUILTINS_GLOBAL_CALL_JS_BOUND_FUNCTION = 0,
    BUILTINS_GLOBAL_CALL_JS_PROXY,
    BUILTINS_OBJECT_CREATE_DATA_PROPERTY_ON_OBJECT_FUNCTIONS,
    BUILTINS_COLLATOR_ANONYMOUS_COLLATOR,
    BUILTINS_DATE_TIME_FORMAT_ANONYMOUS_DATE_TIME_FORMAT,
    BUILTINS_NUMBER_FORMAT_NUMBER_FORMAT_INTERNAL_FORMAT_NUMBER,
    BUILTINS_PROXY_INVALIDATE_PROXY_FUNCTION,
    BUILTINS_PROMISE_HANDLER_ASYNC_AWAIT_FULFILLED,
    BUILTINS_PROMISE_HANDLER_ASYNC_AWAIT_REJECTED,
    BUILTINS_PROMISE_HANDLER_RESOLVE_ELEMENT_FUNCTION,
    BUILTINS_PROMISE_HANDLER_RESOLVE,
    BUILTINS_PROMISE_HANDLER_REJECT,
    BUILTINS_PROMISE_HANDLER_EXECUTOR,
    BUILTINS_PROMISE_HANDLER_ANY_REJECT_ELEMENT_FUNCTION,
    BUILTINS_PROMISE_HANDLER_ALL_SETTLED_RESOLVE_ELEMENT_FUNCTION,
    BUILTINS_PROMISE_HANDLER_ALL_SETTLED_REJECT_ELEMENT_FUNCTION,
    BUILTINS_PROMISE_HANDLER_THEN_FINALLY_FUNCTION,
    BUILTINS_PROMISE_HANDLER_CATCH_FINALLY_FUNCTION,
    BUILTINS_PROMISE_HANDLER_VALUE_THUNK_FUNCTION,
    BUILTINS_PROMISE_HANDLER_THROWER_FUNCTION,
    BUILTINS_ASYNC_GENERATOR_NEXT_FULFILLED_FUNCTION,
    BUILTINS_ASYNC_GENERATOR_NEXT_REJECTED_FUNCTION,
    BUILTINS_ASYNC_FROM_SYNC_ITERATOR_FUNCTION,
    BUILTINS_ASYNC_MODULE_FULFILLED_FUNCTION,
    BUILTINS_ASYNC_MODULE_REJECTED_FUNCTION,
    METHOD_END
};
}  // namespace panda::ecmascript
#endif  // ECMASCRIPT_BUILTINS_BUILTINS_METHOD_INDEX_H