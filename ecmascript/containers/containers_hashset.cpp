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

#include "containers_hashset.h"
#include "ecmascript/ecma_vm.h"
#include "ecmascript/interpreter/interpreter.h"
#include "ecmascript/js_api/js_api_hashset.h"
#include "ecmascript/js_api/js_api_hashset_iterator.h"
#include "ecmascript/js_function.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/tagged_array-inl.h"
#include "ecmascript/tagged_hash_array.h"

namespace panda::ecmascript::containers {
JSTaggedValue ContainersHashSet::HashSetConstructor(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashSet, Constructor);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<JSTaggedValue> newTarget = GetNewTarget(argv);
    if (newTarget->IsUndefined()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "new target can't be undefined", JSTaggedValue::Exception());
    }

    JSHandle<JSTaggedValue> constructor = GetConstructor(argv);
    JSHandle<JSObject> obj = factory->NewJSObjectByConstructor(JSHandle<JSFunction>(constructor), newTarget);
    RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread);

    JSHandle<JSAPIHashSet> hashSet = JSHandle<JSAPIHashSet>::Cast(obj);
    JSTaggedValue hashSetArray = TaggedHashArray::Create(thread);
    hashSet->SetTable(thread, hashSetArray);
    hashSet->SetSize(0);

    return hashSet.GetTaggedValue();
}

JSTaggedValue ContainersHashSet::Values(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashSet, Values);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> self = GetThis(argv);
    if (!self->IsJSAPIHashSet()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashSet", JSTaggedValue::Exception());
    }
    JSHandle<JSTaggedValue> iter = JSAPIHashSetIterator::CreateHashSetIterator(thread, self, IterationKind::VALUE);
    return iter.GetTaggedValue();
}

JSTaggedValue ContainersHashSet::Entries(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashSet, Entries);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> self = GetThis(argv);
    if (!self->IsJSAPIHashSet()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashSet", JSTaggedValue::Exception());
    }
    JSHandle<JSTaggedValue> iter =
        JSAPIHashSetIterator::CreateHashSetIterator(thread, self, IterationKind::KEY_AND_VALUE);
    return iter.GetTaggedValue();
}

JSTaggedValue ContainersHashSet::Add(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashSet, Add);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> self = GetThis(argv);

    if (!self->IsJSAPIHashSet()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashSet", JSTaggedValue::Exception());
    }

    JSHandle<JSTaggedValue> value = GetCallArg(argv, 0);
    JSHandle<JSAPIHashSet> hashSet = JSHandle<JSAPIHashSet>::Cast(self);
    JSAPIHashSet::Add(thread, hashSet, value);
    RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread);
    return hashSet.GetTaggedValue();
}

JSTaggedValue ContainersHashSet::Remove(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashSet, Remove);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> self = GetThis(argv);

    if (!self->IsJSAPIHashSet()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashSet", JSTaggedValue::Exception());
    }

    JSHandle<JSTaggedValue> key = GetCallArg(argv, 0);
    JSHandle<JSAPIHashSet> hashSet = JSHandle<JSAPIHashSet>::Cast(self);
    return JSAPIHashSet::Remove(thread, hashSet, key.GetTaggedValue());
}

JSTaggedValue ContainersHashSet::Has(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashSet, Has);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> self = GetThis(argv);

    if (!self->IsJSAPIHashSet()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashSet", JSTaggedValue::Exception());
    }
    JSHandle<JSTaggedValue> value = GetCallArg(argv, 0);
    JSHandle<JSAPIHashSet> jsHashSet = JSHandle<JSAPIHashSet>::Cast(self);
    return jsHashSet->Has(thread, value.GetTaggedValue());
}

JSTaggedValue ContainersHashSet::Clear(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashSet, Clear);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> self = GetThis(argv);

    if (!self->IsJSAPIHashSet()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashSet", JSTaggedValue::Exception());
    }
    JSHandle<JSAPIHashSet> jsHashSet = JSHandle<JSAPIHashSet>::Cast(self);
    jsHashSet->Clear(thread);
    return JSTaggedValue::Undefined();
}

JSTaggedValue ContainersHashSet::GetLength(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashSet, GetLength);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> self = GetThis(argv);

    if (!self->IsJSAPIHashSet()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashSet", JSTaggedValue::Exception());
    }

    JSHandle<JSAPIHashSet> jsHashSet = JSHandle<JSAPIHashSet>::Cast(self);
    return jsHashSet->GetLength();
}

JSTaggedValue ContainersHashSet::IsEmpty(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashSet, IsEmpty);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> self = GetThis(argv);

    if (!self->IsJSAPIHashSet()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashSet", JSTaggedValue::Exception());
    }
    JSHandle<JSAPIHashSet> jsHashSet = JSHandle<JSAPIHashSet>::Cast(self);
    return jsHashSet->IsEmpty();
}

JSTaggedValue ContainersHashSet::ForEach(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv != nullptr);
    JSThread *thread = argv->GetThread();
    BUILTINS_API_TRACE(thread, HashSet, ForEach);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSTaggedValue> thisHandle = GetThis(argv);
    if (!thisHandle->IsJSAPIHashSet()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not JSAPIHashSet", JSTaggedValue::Exception());
    }
    JSHandle<JSTaggedValue> callbackFnHandle = GetCallArg(argv, 0);
    if (!callbackFnHandle->IsCallable()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "the callbackfun is not callable", JSTaggedValue::Exception());
    }
    JSHandle<JSTaggedValue> thisArgHandle = GetCallArg(argv, 1);
    JSHandle<JSAPIHashSet> hashSet = JSHandle<JSAPIHashSet>::Cast(thisHandle);
    JSHandle<TaggedHashArray> table(thread, hashSet->GetTable());
    uint32_t len = table->GetLength();
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSMutableHandle<TaggedQueue> queue(thread, factory->NewTaggedQueue(0));
    JSMutableHandle<TaggedNode> node(thread, JSTaggedValue::Undefined());
    JSMutableHandle<JSTaggedValue> currentKey(thread, JSTaggedValue::Undefined());
    uint32_t index = 0;
    JSHandle<JSTaggedValue> undefined = thread->GlobalConstants()->GetHandledUndefined();
    while (index < len) {
        node.Update(TaggedHashArray::GetCurrentNode(thread, queue, table, index));
        if (!node.GetTaggedValue().IsHole()) {
            currentKey.Update(node->GetKey());
            EcmaRuntimeCallInfo *info =
                EcmaInterpreter::NewRuntimeCallInfo(thread, callbackFnHandle,
                                                    thisArgHandle, undefined, 2); // 2: two args
            RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread);
            info->SetCallArg(currentKey.GetTaggedValue(), thisHandle.GetTaggedValue());
            JSTaggedValue funcResult = JSFunction::Call(info);
            RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, funcResult);
        }
    }
    return JSTaggedValue::Undefined();
}
} // namespace panda::ecmascript::containers
