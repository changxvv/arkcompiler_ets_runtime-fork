/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "ecmascript/debugger/debugger_api.h"

#include "ecmascript/base/number_helper.h"
#include "ecmascript/debugger/js_debugger.h"
#include "ecmascript/ecma_macros.h"
#include "ecmascript/interpreter/frame_handler.h"
#include "ecmascript/interpreter/slow_runtime_stub.h"
#include "ecmascript/interpreter/fast_runtime_stub-inl.h"
#include "ecmascript/jspandafile/js_pandafile_executor.h"
#include "ecmascript/jspandafile/program_object.h"
#include "ecmascript/js_handle.h"
#include "ecmascript/jspandafile/js_pandafile_manager.h"
#include "ecmascript/method.h"
#include "ecmascript/module/js_module_manager.h"
#include "ecmascript/module/js_module_source_text.h"
#include "ecmascript/napi/jsnapi_helper.h"
#include "ecmascript/tagged_array.h"
#include "ecmascript/tagged_dictionary.h"
#include "ecmascript/tagged_hash_array.h"
#include "ecmascript/tagged_tree.h"
#include "ecmascript/tagged_queue.h"
#include "ecmascript/js_api/js_api_hashmap.h"
#include "ecmascript/js_api/js_api_hashset.h"
#include "ecmascript/js_api/js_api_tree_map.h"
#include "ecmascript/js_api/js_api_tree_set.h"
#include "ecmascript/js_api/js_api_lightweightmap.h"
#include "ecmascript/js_api/js_api_lightweightset.h"

namespace panda::ecmascript::tooling {
using panda::ecmascript::base::ALLOW_BINARY;
using panda::ecmascript::base::ALLOW_HEX;
using panda::ecmascript::base::ALLOW_OCTAL;
using panda::ecmascript::base::NumberHelper;
using ecmascript::JSAPIArrayList;
using ecmascript::JSAPIDeque;
using ecmascript::JSAPIHashMap;
using ecmascript::JSAPIHashSet;
using ecmascript::JSAPILightWeightMap;
using ecmascript::JSAPILightWeightSet;
using ecmascript::JSAPIList;
using ecmascript::JSAPILinkedList;
using ecmascript::JSAPIPlainArray;
using ecmascript::JSAPIStack;
using ecmascript::JSAPIQueue;
using ecmascript::JSAPITreeSet;
using ecmascript::JSAPITreeMap;
using ecmascript::JSAPIVector;
using ecmascript::LinkedNode;
using ecmascript::TaggedHashArray;
using ecmascript::TaggedNode;
using ecmascript::RBTreeNode;
using ecmascript::TaggedTreeSet;
using ecmascript::TaggedTreeMap;
using ecmascript::TaggedQueue;

// FrameHandler
uint32_t DebuggerApi::GetStackDepth(const EcmaVM *ecmaVm)
{
    uint32_t count = 0;
    FrameHandler frameHandler(ecmaVm->GetJSThread());
    for (; frameHandler.HasFrame(); frameHandler.PrevJSFrame()) {
        if (frameHandler.IsEntryFrame() || frameHandler.IsBuiltinFrame()) {
            continue;
        }
        ++count;
    }
    return count;
}

std::shared_ptr<FrameHandler> DebuggerApi::NewFrameHandler(const EcmaVM *ecmaVm)
{
    return std::make_shared<FrameHandler>(ecmaVm->GetJSThread());
}

bool DebuggerApi::StackWalker(const EcmaVM *ecmaVm, std::function<StackState(const FrameHandler *)> func)
{
    FrameHandler frameHandler(ecmaVm->GetJSThread());
    for (; frameHandler.HasFrame(); frameHandler.PrevJSFrame()) {
        if (frameHandler.IsEntryFrame() || frameHandler.IsBuiltinFrame()) {
            continue;
        }
        StackState state = func(&frameHandler);
        if (state == StackState::CONTINUE) {
            continue;
        }
        if (state == StackState::FAILED) {
            return false;
        }
        return true;
    }
    return true;
}

uint32_t DebuggerApi::GetBytecodeOffset(const EcmaVM *ecmaVm)
{
    return FrameHandler(ecmaVm->GetJSThread()).GetBytecodeOffset();
}

std::unique_ptr<PtMethod> DebuggerApi::GetMethod(const EcmaVM *ecmaVm)
{
    FrameHandler frameHandler(ecmaVm->GetJSThread());
    Method* method = frameHandler.GetMethod();
    std::unique_ptr<PtMethod> ptMethod = std::make_unique<PtMethod>(
        method->GetJSPandaFile(), method->GetMethodId(), method->IsNativeWithCallField());
    return ptMethod;
}

void DebuggerApi::SetVRegValue(FrameHandler *frameHandler, size_t index, Local<JSValueRef> value)
{
    return frameHandler->SetVRegValue(index, JSNApiHelper::ToJSTaggedValue(*value));
}

uint32_t DebuggerApi::GetBytecodeOffset(const FrameHandler *frameHandler)
{
    return frameHandler->GetBytecodeOffset();
}

Method *DebuggerApi::GetMethod(const FrameHandler *frameHandler)
{
    return frameHandler->GetMethod();
}

bool DebuggerApi::IsNativeMethod(const EcmaVM *ecmaVm)
{
    FrameHandler frameHandler(ecmaVm->GetJSThread());
    return DebuggerApi::IsNativeMethod(&frameHandler);
}

bool DebuggerApi::IsNativeMethod(const FrameHandler *frameHandler)
{
    if (!frameHandler->HasFrame()) {
        return false;
    }
    Method* method = frameHandler->GetMethod();
    return method->IsNativeWithCallField();
}

JSPandaFile *DebuggerApi::GetJSPandaFile(const EcmaVM *ecmaVm)
{
    Method *method = FrameHandler(ecmaVm->GetJSThread()).GetMethod();
    return const_cast<JSPandaFile *>(method->GetJSPandaFile());
}

JSTaggedValue DebuggerApi::GetEnv(const FrameHandler *frameHandler)
{
    return frameHandler->GetEnv();
}

JSTaggedType *DebuggerApi::GetSp(const FrameHandler *frameHandler)
{
    return frameHandler->GetSp();
}

int32_t DebuggerApi::GetVregIndex(const FrameHandler *frameHandler, std::string_view name)
{
    Method *method = DebuggerApi::GetMethod(frameHandler);
    if (method->IsNativeWithCallField()) {
        LOG_DEBUGGER(ERROR) << "GetVregIndex: native frame not support";
        return -1;
    }
    DebugInfoExtractor *extractor = JSPandaFileManager::GetInstance()->GetJSPtExtractor(method->GetJSPandaFile());
    if (extractor == nullptr) {
        LOG_DEBUGGER(ERROR) << "GetVregIndex: extractor is null";
        return -1;
    }
    auto table = extractor->GetLocalVariableTable(method->GetMethodId());
    for (auto iter = table.begin(); iter != table.end(); iter++) {
        if (iter->name == name.data()) {
            return iter->regNumber;
        }
    }
    return -1;
}

Local<JSValueRef> DebuggerApi::GetVRegValue(const EcmaVM *ecmaVm,
    const FrameHandler *frameHandler, size_t index)
{
    auto value = frameHandler->GetVRegValue(index);
    JSHandle<JSTaggedValue> handledValue(ecmaVm->GetJSThread(), value);
    return JSNApiHelper::ToLocal<JSValueRef>(handledValue);
}

// JSThread
Local<JSValueRef> DebuggerApi::GetAndClearException(const EcmaVM *ecmaVm)
{
    auto exception = ecmaVm->GetJSThread()->GetException();
    JSHandle<JSTaggedValue> handledException(ecmaVm->GetJSThread(), exception);
    ecmaVm->GetJSThread()->ClearException();
    return JSNApiHelper::ToLocal<JSValueRef>(handledException);
}

void DebuggerApi::SetException(const EcmaVM *ecmaVm, Local<JSValueRef> exception)
{
    ecmaVm->GetJSThread()->SetException(JSNApiHelper::ToJSTaggedValue(*exception));
}

void DebuggerApi::ClearException(const EcmaVM *ecmaVm)
{
    return ecmaVm->GetJSThread()->ClearException();
}

// NumberHelper
double DebuggerApi::StringToDouble(const uint8_t *start, const uint8_t *end, uint8_t radix)
{
    return NumberHelper::StringToDouble(start, end, radix, ALLOW_BINARY | ALLOW_HEX | ALLOW_OCTAL);
}

// JSDebugger
JSDebugger *DebuggerApi::CreateJSDebugger(const EcmaVM *ecmaVm)
{
    return new JSDebugger(ecmaVm);
}

void DebuggerApi::DestroyJSDebugger(JSDebugger *debugger)
{
    delete debugger;
}

void DebuggerApi::RegisterHooks(JSDebugger *debugger, PtHooks *hooks)
{
    debugger->RegisterHooks(hooks);
}

bool DebuggerApi::SetBreakpoint(JSDebugger *debugger, const JSPtLocation &location,
    Local<FunctionRef> condFuncRef)
{
    return debugger->SetBreakpoint(location, condFuncRef);
}

bool DebuggerApi::RemoveBreakpoint(JSDebugger *debugger, const JSPtLocation &location)
{
    return debugger->RemoveBreakpoint(location);
}

void DebuggerApi::RemoveAllBreakpoints(JSDebugger *debugger)
{
    return debugger->RemoveAllBreakpoints();
}

// ScopeInfo
Local<JSValueRef> DebuggerApi::GetProperties(const EcmaVM *ecmaVm, const FrameHandler *frameHandler,
                                             int32_t level, uint32_t slot)
{
    JSTaggedValue env = frameHandler->GetEnv();
    for (int i = 0; i < level; i++) {
        JSTaggedValue taggedParentEnv = LexicalEnv::Cast(env.GetTaggedObject())->GetParentEnv();
        ASSERT(!taggedParentEnv.IsUndefined());
        env = taggedParentEnv;
    }
    JSTaggedValue value = LexicalEnv::Cast(env.GetTaggedObject())->GetProperties(slot);
    JSHandle<JSTaggedValue> handledValue(ecmaVm->GetJSThread(), value);
    return JSNApiHelper::ToLocal<JSValueRef>(handledValue);
}

void DebuggerApi::SetProperties(const EcmaVM *ecmaVm, const FrameHandler *frameHandler,
                                int32_t level, uint32_t slot, Local<JSValueRef> value)
{
    JSTaggedValue env = frameHandler->GetEnv();
    for (int i = 0; i < level; i++) {
        JSTaggedValue taggedParentEnv = LexicalEnv::Cast(env.GetTaggedObject())->GetParentEnv();
        ASSERT(!taggedParentEnv.IsUndefined());
        env = taggedParentEnv;
    }
    JSTaggedValue target = JSNApiHelper::ToJSHandle(value).GetTaggedValue();
    LexicalEnv::Cast(env.GetTaggedObject())->SetProperties(ecmaVm->GetJSThread(), slot, target);
}

std::pair<int32_t, uint32_t> DebuggerApi::GetLevelSlot(const FrameHandler *frameHandler, std::string_view name)
{
    int32_t level = 0;
    uint32_t slot = 0;
    JSTaggedValue curEnv = frameHandler->GetEnv();
    for (; curEnv.IsTaggedArray(); curEnv = LexicalEnv::Cast(curEnv.GetTaggedObject())->GetParentEnv(), level++) {
        LexicalEnv *lexicalEnv = LexicalEnv::Cast(curEnv.GetTaggedObject());
        if (lexicalEnv->GetScopeInfo().IsHole()) {
            continue;
        }
        auto result = JSNativePointer::Cast(lexicalEnv->GetScopeInfo().GetTaggedObject())->GetExternalPointer();
        ScopeDebugInfo *scopeDebugInfo = reinterpret_cast<ScopeDebugInfo *>(result);
        auto iter = scopeDebugInfo->scopeInfo.find(name.data());
        if (iter == scopeDebugInfo->scopeInfo.end()) {
            continue;
        }
        slot = iter->second;
        return std::make_pair(level, slot);
    }
    return std::make_pair(-1, 0);
}

Local<JSValueRef> DebuggerApi::GetGlobalValue(const EcmaVM *ecmaVm, Local<StringRef> name)
{
    JSTaggedValue result;
    JSTaggedValue globalObj = ecmaVm->GetGlobalEnv()->GetGlobalObject();
    JSThread *thread = ecmaVm->GetJSThread();

    JSTaggedValue key = JSNApiHelper::ToJSTaggedValue(*name);
    JSTaggedValue globalRec = SlowRuntimeStub::LdGlobalRecord(thread, key);
    if (!globalRec.IsUndefined()) {
        ASSERT(globalRec.IsPropertyBox());
        result = PropertyBox::Cast(globalRec.GetTaggedObject())->GetValue();
        return JSNApiHelper::ToLocal<JSValueRef>(JSHandle<JSTaggedValue>(thread, result));
    }

    JSTaggedValue globalVar = FastRuntimeStub::GetGlobalOwnProperty(thread, globalObj, key);
    if (!globalVar.IsHole()) {
        return JSNApiHelper::ToLocal<JSValueRef>(JSHandle<JSTaggedValue>(thread, globalVar));
    } else {
        result = SlowRuntimeStub::TryLdGlobalByNameFromGlobalProto(thread, globalObj, key);
        return JSNApiHelper::ToLocal<JSValueRef>(JSHandle<JSTaggedValue>(thread, result));
    }

    return Local<JSValueRef>();
}

bool DebuggerApi::SetGlobalValue(const EcmaVM *ecmaVm, Local<StringRef> name, Local<JSValueRef> value)
{
    JSTaggedValue result;
    JSTaggedValue globalObj = ecmaVm->GetGlobalEnv()->GetGlobalObject();
    JSThread *thread = ecmaVm->GetJSThread();

    JSTaggedValue key = JSNApiHelper::ToJSTaggedValue(*name);
    JSTaggedValue newVal = JSNApiHelper::ToJSTaggedValue(*value);
    JSTaggedValue globalRec = SlowRuntimeStub::LdGlobalRecord(thread, key);
    if (!globalRec.IsUndefined()) {
        result = SlowRuntimeStub::TryUpdateGlobalRecord(thread, key, newVal);
        return !result.IsException();
    }

    JSTaggedValue globalVar = FastRuntimeStub::GetGlobalOwnProperty(thread, globalObj, key);
    if (!globalVar.IsHole()) {
        result = SlowRuntimeStub::StGlobalVar(thread, key, newVal);
        return !result.IsException();
    }

    return false;
}

JSTaggedValue DebuggerApi::GetCurrentModule(const EcmaVM *ecmaVm)
{
    JSThread *thread = ecmaVm->GetJSThread();
    FrameHandler frameHandler(thread);
    for (; frameHandler.HasFrame(); frameHandler.PrevJSFrame()) {
        if (frameHandler.IsEntryFrame()) {
            continue;
        }
        Method *method = frameHandler.GetMethod();
        // Skip builtins method
        if (method->IsNativeWithCallField()) {
            continue;
        }
        JSTaggedValue func = frameHandler.GetFunction();
        JSTaggedValue module = JSFunction::Cast(func.GetTaggedObject())->GetModule();
        if (module.IsUndefined()) {
            continue;
        }
        return module;
    }
    UNREACHABLE();
}

JSHandle<JSTaggedValue> DebuggerApi::GetImportModule(const EcmaVM *ecmaVm,
                                                     const JSHandle<JSTaggedValue> &currentModule, std::string &name)
{
    JSTaggedValue importEntries = SourceTextModule::Cast(currentModule->GetTaggedObject())->GetImportEntries();
    if (importEntries.IsUndefined()) {
        return currentModule;
    }

    JSThread *thread = ecmaVm->GetJSThread();
    JSHandle<TaggedArray> importArray(thread, TaggedArray::Cast(importEntries.GetTaggedObject()));
    size_t importEntriesLen = importArray->GetLength();
    JSHandle<JSTaggedValue> starString = thread->GlobalConstants()->GetHandledStarString();
    JSMutableHandle<ImportEntry> ee(thread, thread->GlobalConstants()->GetUndefined());
    JSMutableHandle<TaggedArray> environment(thread, thread->GlobalConstants()->GetUndefined());
    JSMutableHandle<JSTaggedValue> importModule(thread, thread->GlobalConstants()->GetUndefined());
    for (size_t idx = 0; idx < importEntriesLen; idx++) {
        ee.Update(importArray->Get(idx));
        JSTaggedValue localName = ee->GetLocalName();
        JSTaggedValue importName = ee->GetImportName();
        // Skip 'import * as name from xxx'
        if (localName.IsString() && !JSTaggedValue::SameValue(importName, starString.GetTaggedValue())) {
            std::string varName = EcmaStringAccessor(localName).ToStdString();
            if (varName != name) {
                continue;
            }
            JSTaggedValue moduleEnvironment = SourceTextModule::Cast(
                currentModule->GetTaggedObject())->GetEnvironment();
            environment.Update(moduleEnvironment);
            JSTaggedValue resolvedBinding = environment->Get(idx);
            ResolvedIndexBinding *binding = ResolvedIndexBinding::Cast(resolvedBinding.GetTaggedObject());
            importModule.Update(binding->GetModule());
            name = EcmaStringAccessor(importName).ToStdString();
            return importModule;
        }
    }
    return currentModule;
}

int32_t DebuggerApi::GetModuleVariableIndex(const EcmaVM *ecmaVm, const JSHandle<JSTaggedValue> &currentModule,
                                            std::string &name)
{
    JSTaggedValue dictionary = SourceTextModule::Cast(currentModule->GetTaggedObject())->GetNameDictionary();
    if (dictionary.IsUndefined()) {
        return -1;
    }

    JSThread *thread = ecmaVm->GetJSThread();
    if (dictionary.IsTaggedArray()) {
        JSTaggedValue localExportEntries = SourceTextModule::Cast(
            currentModule->GetTaggedObject())->GetLocalExportEntries();
        ASSERT(localExportEntries.IsTaggedArray());
        JSHandle<TaggedArray> localExportArray(thread, TaggedArray::Cast(localExportEntries.GetTaggedObject()));
        uint32_t exportEntriesLen = localExportArray->GetLength();
        JSMutableHandle<LocalExportEntry> ee(thread, thread->GlobalConstants()->GetUndefined());
        for (uint32_t idx = 0; idx < exportEntriesLen; idx++) {
            ee.Update(localExportArray->Get(idx));
            JSTaggedValue localKey = ee->GetLocalName();
            JSTaggedValue exportKey = ee->GetExportName();
            if (localKey.IsString() && exportKey.IsString()) {
                std::string localName = EcmaStringAccessor(localKey).ToStdString();
                std::string exportName = EcmaStringAccessor(exportKey).ToStdString();
                if (localName == name || exportName == name) {
                    return idx;
                }
            }
        }
    }
    return -1;
}

int32_t DebuggerApi::GetRequestModuleIndex(const EcmaVM *ecmaVm, JSTaggedValue moduleRequest,
                                           const JSHandle<JSTaggedValue> &currentModule)
{
    JSThread *thread = ecmaVm->GetJSThread();
    JSHandle<SourceTextModule> module(thread, SourceTextModule::Cast(currentModule->GetTaggedObject()));
    JSHandle<JSTaggedValue> required(thread, moduleRequest);
    JSHandle<TaggedArray> requestedModules(thread, module->GetRequestedModules());
    uint32_t requestedModulesLen = requestedModules->GetLength();
    for (uint32_t idx = 0; idx < requestedModulesLen; idx++) {
        JSTaggedValue requestModule = requestedModules->Get(idx);
        if (JSTaggedValue::SameValue(required.GetTaggedValue(), requestModule)) {
            return idx;
        }
    }
    LOG_ECMA(FATAL) << "this branch is unreachable";
    return -1;
}

Local<JSValueRef> DebuggerApi::GetModuleValue(const EcmaVM *ecmaVm, const JSHandle<JSTaggedValue> &currentModule,
                                              std::string &name)
{
    Local<JSValueRef> result;
    JSHandle<JSTaggedValue> module = GetImportModule(ecmaVm, currentModule, name);
    int32_t index = GetModuleVariableIndex(ecmaVm, module, name);
    if (index == -1) {
        return result;
    }

    JSTaggedValue dictionary = SourceTextModule::Cast(module->GetTaggedObject())->GetNameDictionary();
    if (dictionary.IsUndefined()) {
        return result;
    }

    JSThread *thread = ecmaVm->GetJSThread();
    if (dictionary.IsTaggedArray()) {
        TaggedArray *array = TaggedArray::Cast(dictionary.GetTaggedObject());
        JSTaggedValue moduleValue = array->Get(index);
        result = JSNApiHelper::ToLocal<JSValueRef>(JSHandle<JSTaggedValue>(thread, moduleValue));
        return result;
    }
    return result;
}

bool DebuggerApi::SetModuleValue(const EcmaVM *ecmaVm, const JSHandle<JSTaggedValue> &currentModule,
                                 std::string &name, Local<JSValueRef> value)
{
    JSHandle<JSTaggedValue> module = GetImportModule(ecmaVm, currentModule, name);
    int32_t index = GetModuleVariableIndex(ecmaVm, module, name);
    if (index == -1) {
        return false;
    }

    JSTaggedValue dictionary = SourceTextModule::Cast(module->GetTaggedObject())->GetNameDictionary();
    if (dictionary.IsUndefined()) {
        return false;
    }

    JSThread *thread = ecmaVm->GetJSThread();
    JSTaggedValue curValue = JSNApiHelper::ToJSTaggedValue(*value);
    if (dictionary.IsTaggedArray()) {
        TaggedArray *array = TaggedArray::Cast(dictionary.GetTaggedObject());
        array->Set(thread, index, curValue);
    }
    return true;
}

void DebuggerApi::InitializeExportVariables(const EcmaVM *ecmaVm, Local<ObjectRef> &moduleObj,
                                            const JSHandle<JSTaggedValue> &currentModule)
{
    JSTaggedValue localExportEntries = SourceTextModule::Cast(
        currentModule->GetTaggedObject())->GetLocalExportEntries();
    if (localExportEntries.IsUndefined()) {
        return;
    }

    JSThread *thread = ecmaVm->GetJSThread();
    JSHandle<TaggedArray> localExportArray(thread, TaggedArray::Cast(localExportEntries.GetTaggedObject()));
    uint32_t exportEntriesLen = localExportArray->GetLength();
    JSMutableHandle<LocalExportEntry> ee(thread, thread->GlobalConstants()->GetUndefined());
    JSMutableHandle<JSTaggedValue> name(thread, thread->GlobalConstants()->GetUndefined());
    JSMutableHandle<JSTaggedValue> value(thread, thread->GlobalConstants()->GetUndefined());
    JSTaggedValue moduleValue = JSTaggedValue::Undefined();
    for (uint32_t idx = 0; idx < exportEntriesLen; idx++) {
        ee.Update(localExportArray->Get(idx));
        JSTaggedValue key = ee->GetLocalName();
        name.Update(key);
        value.Update(moduleValue);
        if (key.IsString()) {
            Local<JSValueRef> variableName = JSNApiHelper::ToLocal<JSValueRef>(name);
            Local<JSValueRef> variableValue = JSNApiHelper::ToLocal<JSValueRef>(value);
            PropertyAttribute descriptor(variableValue, true, true, true);
            moduleObj->DefineProperty(ecmaVm, variableName, descriptor);
        }
    }
}

void DebuggerApi::GetLocalExportVariables(const EcmaVM *ecmaVm, Local<ObjectRef> &moduleObj,
                                          const JSHandle<JSTaggedValue> &currentModule, bool isImportStar)
{
    JSTaggedValue dictionary = SourceTextModule::Cast(currentModule->GetTaggedObject())->GetNameDictionary();
    if (dictionary.IsUndefined()) {
        InitializeExportVariables(ecmaVm, moduleObj, currentModule);
        return;
    }

    JSThread *thread = ecmaVm->GetJSThread();
    JSMutableHandle<JSTaggedValue> name(thread, thread->GlobalConstants()->GetUndefined());
    JSMutableHandle<JSTaggedValue> value(thread, thread->GlobalConstants()->GetUndefined());
    if (dictionary.IsTaggedArray()) {
        JSTaggedValue localExportEntries = SourceTextModule::Cast(
            currentModule->GetTaggedObject())->GetLocalExportEntries();
        ASSERT(localExportEntries.IsTaggedArray());
        JSHandle<TaggedArray> localExportArray(thread, TaggedArray::Cast(localExportEntries.GetTaggedObject()));
        uint32_t exportEntriesLen = localExportArray->GetLength();
        JSHandle<TaggedArray> dict(thread, TaggedArray::Cast(dictionary.GetTaggedObject()));
        uint32_t valueLen = dict->GetLength();
        if (exportEntriesLen != valueLen) {
            LOG_FULL(FATAL) << "Key does not match value";
        }

        JSMutableHandle<LocalExportEntry> ee(thread, thread->GlobalConstants()->GetUndefined());
        for (uint32_t idx = 0; idx < exportEntriesLen; idx++) {
            ee.Update(localExportArray->Get(idx));
            JSTaggedValue key;
            if (isImportStar) {
                key = ee->GetExportName();
            } else {
                key = ee->GetLocalName();
            }
            name.Update(key);
            JSTaggedValue moduleValue = dict->Get(idx);
            if (moduleValue.IsHole()) {
                moduleValue = JSTaggedValue::Undefined();
            }
            value.Update(moduleValue);
            if (key.IsString()) {
                Local<JSValueRef> variableName = JSNApiHelper::ToLocal<JSValueRef>(name);
                Local<JSValueRef> variableValue = JSNApiHelper::ToLocal<JSValueRef>(value);
                PropertyAttribute descriptor(variableValue, true, true, true);
                moduleObj->DefineProperty(ecmaVm, variableName, descriptor);
            }
        }
    }
}

void DebuggerApi::GetIndirectExportVariables(const EcmaVM *ecmaVm, Local<ObjectRef> &moduleObj,
                                             const JSHandle<JSTaggedValue> &currentModule)
{
    JSTaggedValue indirectExportEntries = SourceTextModule::Cast(
        currentModule->GetTaggedObject())->GetIndirectExportEntries();
    if (indirectExportEntries.IsUndefined()) {
        return;
    }
    ASSERT(indirectExportEntries.IsTaggedArray());
    JSThread *thread = ecmaVm->GetJSThread();
    JSHandle<TaggedArray> indirectExportArray(thread, TaggedArray::Cast(indirectExportEntries.GetTaggedObject()));
    uint32_t indirectExportEntriesLen = indirectExportArray->GetLength();
    JSMutableHandle<IndirectExportEntry> ee(thread, thread->GlobalConstants()->GetUndefined());
    JSMutableHandle<JSTaggedValue> name(thread, thread->GlobalConstants()->GetUndefined());
    for (uint32_t idx = 0; idx < indirectExportEntriesLen; idx++) {
        ee.Update(indirectExportArray->Get(idx));
        JSTaggedValue key = ee->GetImportName();
        name.Update(key);
        if (key.IsString()) {
            Local<JSValueRef> variableName = JSNApiHelper::ToLocal<JSValueRef>(name);
            JSHandle<JSTaggedValue> moduleRequest(thread, ee->GetModuleRequest());
            JSHandle<JSTaggedValue> importModule;
            JSHandle<SourceTextModule> module = JSHandle<SourceTextModule>::Cast(currentModule);
            JSTaggedValue moduleRecordName = module->GetEcmaModuleRecordName();
            if (moduleRecordName.IsUndefined()) {
                importModule = SourceTextModule::HostResolveImportedModule(thread, module, moduleRequest);
            } else {
                importModule = SourceTextModule::HostResolveImportedModuleWithMerge(thread, module, moduleRequest);
            }
            std::string importName = EcmaStringAccessor(ee->GetImportName()).ToStdString();
            Local<JSValueRef> value = GetModuleValue(ecmaVm, importModule, importName);
            PropertyAttribute descriptor(value, true, true, true);
            moduleObj->DefineProperty(ecmaVm, variableName, descriptor);
        }
    }
}

void DebuggerApi::GetImportVariables(const EcmaVM *ecmaVm, Local<ObjectRef> &moduleObj,
                                     const JSHandle<JSTaggedValue> &currentModule)
{
    JSTaggedValue importEntries = SourceTextModule::Cast(currentModule->GetTaggedObject())->GetImportEntries();
    if (importEntries.IsUndefined()) {
        return;
    }

    JSTaggedValue moduleEnvironment = SourceTextModule::Cast(currentModule->GetTaggedObject())->GetEnvironment();
    if (moduleEnvironment.IsUndefined()) {
        return;
    }

    JSThread *thread = ecmaVm->GetJSThread();
    JSHandle<TaggedArray> importArray(thread, TaggedArray::Cast(importEntries.GetTaggedObject()));
    uint32_t importEntriesLen = importArray->GetLength();
    JSHandle<TaggedArray> environment(thread, TaggedArray::Cast(moduleEnvironment.GetTaggedObject()));
    JSHandle<JSTaggedValue> starString = thread->GlobalConstants()->GetHandledStarString();
    JSMutableHandle<ImportEntry> ee(thread, thread->GlobalConstants()->GetUndefined());
    JSMutableHandle<JSTaggedValue> name(thread, thread->GlobalConstants()->GetUndefined());
    for (uint32_t idx = 0; idx < importEntriesLen; idx++) {
        ee.Update(importArray->Get(idx));
        JSTaggedValue key = ee->GetImportName();
        JSTaggedValue localName = ee->GetLocalName();
        name.Update(localName);
        if (!key.IsString()) {
            continue;
        }
        if (JSTaggedValue::SameValue(key, starString.GetTaggedValue())) {
            JSHandle<JSTaggedValue> moduleRequest(thread, ee->GetModuleRequest());
            JSHandle<JSTaggedValue> importModule;
            JSHandle<SourceTextModule> module = JSHandle<SourceTextModule>::Cast(currentModule);
            JSTaggedValue moduleRecordName = module->GetEcmaModuleRecordName();
            if (moduleRecordName.IsUndefined()) {
                importModule = SourceTextModule::HostResolveImportedModule(thread, module, moduleRequest);
            } else {
                importModule = SourceTextModule::HostResolveImportedModuleWithMerge(thread, module, moduleRequest);
            }
            Local<ObjectRef> importModuleObj = ObjectRef::New(ecmaVm);
            GetLocalExportVariables(ecmaVm, importModuleObj, importModule, true);
            Local<JSValueRef> variableName = JSNApiHelper::ToLocal<JSValueRef>(name);
            PropertyAttribute descriptor(static_cast<Local<JSValueRef>>(importModuleObj), true, true, true);
            moduleObj->DefineProperty(ecmaVm, variableName, descriptor);
            continue;
        }
        JSTaggedValue resolvedBinding = environment->Get(idx);
        if (resolvedBinding.IsHole()) {
            continue;
        }
        ResolvedIndexBinding *binding = ResolvedIndexBinding::Cast(resolvedBinding.GetTaggedObject());
        JSHandle<JSTaggedValue> importModule(thread, binding->GetModule());
        std::string importName = EcmaStringAccessor(key).ToStdString();
        Local<JSValueRef> value = GetModuleValue(ecmaVm, importModule, importName);
        Local<JSValueRef> variableName = JSNApiHelper::ToLocal<JSValueRef>(name);
        PropertyAttribute descriptor(value, true, true, true);
        moduleObj->DefineProperty(ecmaVm, variableName, descriptor);
    }
}

void DebuggerApi::HandleUncaughtException(const EcmaVM *ecmaVm, std::string &message)
{
    JSThread *thread = ecmaVm->GetJSThread();
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    const GlobalEnvConstants *globalConst = thread->GlobalConstants();

    JSHandle<JSTaggedValue> exHandle(thread, thread->GetException());
    thread->ClearException();
    if (exHandle->IsJSError()) {
        JSHandle<JSTaggedValue> nameKey = globalConst->GetHandledNameString();
        JSHandle<EcmaString> name(JSObject::GetProperty(thread, exHandle, nameKey).GetValue());
        JSHandle<JSTaggedValue> msgKey = globalConst->GetHandledMessageString();
        JSHandle<EcmaString> msg(JSObject::GetProperty(thread, exHandle, msgKey).GetValue());
        message = ConvertToString(*name) + ": " + ConvertToString(*msg);
    } else {
        JSHandle<EcmaString> ecmaStr = JSTaggedValue::ToString(thread, exHandle);
        message = ConvertToString(*ecmaStr);
    }
}

Local<FunctionRef> DebuggerApi::GenerateFuncFromBuffer(const EcmaVM *ecmaVm, const void *buffer,
                                                       size_t size, std::string_view entryPoint)
{
    JSPandaFileManager *mgr = JSPandaFileManager::GetInstance();
    std::shared_ptr<JSPandaFile> jsPandaFile =
        mgr->LoadJSPandaFile(ecmaVm->GetJSThread(), "", entryPoint, buffer, size);
    if (jsPandaFile == nullptr) {
        return JSValueRef::Undefined(ecmaVm);
    }

    JSHandle<Program> program = mgr->GenerateProgram(const_cast<EcmaVM *>(ecmaVm), jsPandaFile.get(), entryPoint);
    JSTaggedValue func = program->GetMainFunction();
    return JSNApiHelper::ToLocal<FunctionRef>(JSHandle<JSTaggedValue>(ecmaVm->GetJSThread(), func));
}

Local<JSValueRef> DebuggerApi::EvaluateViaFuncCall(EcmaVM *ecmaVm, Local<FunctionRef> funcRef,
    std::shared_ptr<FrameHandler> &frameHandler)
{
    JSNApi::EnableUserUncaughtErrorHandler(ecmaVm);

    JsDebuggerManager *mgr = ecmaVm->GetJsDebuggerManager();
    bool prevDebugMode = mgr->IsDebugMode();
    mgr->SetEvalFrameHandler(frameHandler);
    mgr->SetDebugMode(false); // in order to catch exception
    ecmaVm->GetJSThread()->CheckSwitchDebuggerBCStub();
    std::vector<Local<JSValueRef>> args;
    auto result = funcRef->Call(ecmaVm, JSValueRef::Undefined(ecmaVm), args.data(), args.size());
    mgr->SetDebugMode(prevDebugMode);
    ecmaVm->GetJSThread()->CheckSwitchDebuggerBCStub();
    mgr->SetEvalFrameHandler(nullptr);

    return result;
}

bool DebuggerApi::IsExceptionCaught(const EcmaVM *ecmaVm)
{
    FrameHandler frameHandler(ecmaVm->GetJSThread());
    for (; frameHandler.HasFrame(); frameHandler.PrevJSFrame()) {
        if (frameHandler.IsEntryFrame()) {
            return false;
        }
        auto method = frameHandler.GetMethod();
        if (method->FindCatchBlock(frameHandler.GetBytecodeOffset() != INVALID_INDEX)) {
            return true;
        }
    }
    return false;
}

DebugInfoExtractor *DebuggerApi::GetPatchExtractor(const EcmaVM *ecmaVm, const std::string &url)
{
    const auto *hotReloadManager = ecmaVm->GetJsDebuggerManager()->GetHotReloadManager();
    return hotReloadManager->GetPatchExtractor(url);
}

const JSPandaFile *DebuggerApi::GetBaseJSPandaFile(const EcmaVM *ecmaVm, const JSPandaFile *jsPandaFile)
{
    const auto *hotReloadManager = ecmaVm->GetJsDebuggerManager()->GetHotReloadManager();
    return hotReloadManager->GetBaseJSPandaFile(jsPandaFile);
}

std::vector<void *> DebuggerApi::GetNativePointer(const EcmaVM *ecmaVm)
{
    void *native = nullptr;
    std::vector<void *> nativePointer;
    JSThread *thread = ecmaVm->GetJSThread();
    JSTaggedType *current = const_cast<JSTaggedType *>(thread->GetCurrentFrame());
    FrameIterator it(current, thread);
    for (; !it.Done(); it.Advance<GCVisitedFlag::HYBRID_STACK>()) {
        if (!it.IsJSFrame()) {
            continue;
        }
        auto method = it.CheckAndGetMethod();
        if (method == nullptr) {
            continue;
        }

        if (method->IsNativeWithCallField()) {
            JSTaggedValue function = it.GetFunction();
            JSHandle<JSTaggedValue> extraInfoValue(
                thread, JSFunction::Cast(function.GetTaggedObject())->GetFunctionExtraInfo());
            auto cb = ecmaVm->GetNativePtrGetter();
            if (extraInfoValue->IsJSNativePointer() && cb != nullptr) {
                JSHandle<JSNativePointer> extraInfo(extraInfoValue);
                native = cb(reinterpret_cast<void *>(extraInfo->GetData()));
                nativePointer.push_back(native);
            }
        } else {
            nativePointer.push_back(nullptr); // to tell IDE this frame don't hava nativePointer
        }
    }
    return nativePointer;
}

uint32_t DebuggerApi::GetContainerLength(const EcmaVM *ecmaVm, Local<JSValueRef> value)
{
    uint32_t length = Local<ArrayRef>(value)->Length(ecmaVm);
    return length;
}

void DebuggerApi::AddInternalProperties(const EcmaVM *ecmaVm, Local<ObjectRef> object,
                                        ArkInternalValueType type, Global<MapRef> internalObjects)
{
    internalObjects->Set(ecmaVm, object, NumberRef::New(ecmaVm, static_cast<int32_t>(type)));
}

Local<JSValueRef> DebuggerApi::GetArrayListValue(const EcmaVM *ecmaVm, Local<JSValueRef> value,
                                                 Global<MapRef> internalObjects)
{
    JSHandle<JSAPIArrayList> arrayList(JSNApiHelper::ToJSHandle(value));
    uint32_t size = static_cast<uint32_t>(arrayList->GetSize());
    Local<JSValueRef> jsValueRef = ArrayRef::New(ecmaVm, size);
    JSThread *thread = ecmaVm->GetJSThread();
    JSMutableHandle<JSTaggedValue> currentValue(thread, JSTaggedValue::Undefined());
    uint32_t index = 0;
    while (index < size) {
        currentValue.Update(arrayList->Get(thread, index));
        ArrayRef::SetValueAt(ecmaVm, jsValueRef, index++, JSNApiHelper::ToLocal<JSValueRef>(currentValue));
    }
    AddInternalProperties(ecmaVm, jsValueRef, ArkInternalValueType::Entry, internalObjects);
    return jsValueRef;
}

Local<JSValueRef> DebuggerApi::GetDequeValue(const EcmaVM *ecmaVm, Local<JSValueRef> value,
                                             Global<MapRef> internalObjects)
{
    JSHandle<JSAPIDeque> deque(JSNApiHelper::ToJSHandle(value));
    uint32_t size = static_cast<uint32_t>(deque->GetSize());
    Local<JSValueRef> jsValueRef = ArrayRef::New(ecmaVm, size);
    JSThread *thread = ecmaVm->GetJSThread();
    JSMutableHandle<JSTaggedValue> currentValue(thread, JSTaggedValue::Undefined());
    uint32_t index = 0;
    while (index < size) {
        currentValue.Update(deque->Get(index));
        ArrayRef::SetValueAt(ecmaVm, jsValueRef, index++, JSNApiHelper::ToLocal<JSValueRef>(currentValue));
    }
    AddInternalProperties(ecmaVm, jsValueRef, ArkInternalValueType::Entry, internalObjects);
    return jsValueRef;
}

Local<JSValueRef> DebuggerApi::GetHashMapValue(const EcmaVM *ecmaVm, Local<JSValueRef> value,
                                               Global<MapRef> internalObjects)
{
    JSHandle<JSAPIHashMap> hashMap(JSNApiHelper::ToJSHandle(value));
    JSThread *thread = ecmaVm->GetJSThread();
    JSHandle<TaggedHashArray> table(thread, hashMap->GetTable());
    uint32_t length = table->GetLength();
    uint32_t size = static_cast<uint32_t>(hashMap->GetSize());
    Local<JSValueRef> jsValueRef = ArrayRef::New(ecmaVm, size);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSMutableHandle<TaggedQueue> queue(thread, factory->NewTaggedQueue(0));
    JSMutableHandle<TaggedNode> node(thread, JSTaggedValue::Undefined());
    JSMutableHandle<JSTaggedValue> currentKey(thread, JSTaggedValue::Undefined());
    JSMutableHandle<JSTaggedValue> currentValue(thread, JSTaggedValue::Undefined());
    Local<JSValueRef> jsKey = StringRef::NewFromUtf8(ecmaVm, "key");
    Local<JSValueRef> jsValue = StringRef::NewFromUtf8(ecmaVm, "value");
    uint32_t pos = 0;
    uint32_t index = 0;
    while (index < length) {
        node.Update(TaggedHashArray::GetCurrentNode(thread, queue, table, index));
        if (!node.GetTaggedValue().IsHole()) {
            currentKey.Update(node->GetKey());
            currentValue.Update(node->GetValue());
            Local<ObjectRef> objRef = ObjectRef::New(ecmaVm);
            objRef->Set(ecmaVm, jsKey, JSNApiHelper::ToLocal<JSValueRef>(currentKey));
            objRef->Set(ecmaVm, jsValue, JSNApiHelper::ToLocal<JSValueRef>(currentValue));
            AddInternalProperties(ecmaVm, objRef, ArkInternalValueType::Entry, internalObjects);
            ArrayRef::SetValueAt(ecmaVm, jsValueRef, pos++, objRef);
        }
    }
    AddInternalProperties(ecmaVm, jsValueRef, ArkInternalValueType::Entry, internalObjects);
    return jsValueRef;
}

Local<JSValueRef> DebuggerApi::GetHashSetValue(const EcmaVM *ecmaVm, Local<JSValueRef> value,
                                               Global<MapRef> internalObjects)
{
    JSHandle<JSAPIHashSet> hashSet(JSNApiHelper::ToJSHandle(value));
    JSThread *thread = ecmaVm->GetJSThread();
    JSHandle<TaggedHashArray> table(thread, hashSet->GetTable());
    uint32_t length = table->GetLength();
    uint32_t size = static_cast<uint32_t>(hashSet->GetSize());
    Local<JSValueRef> jsValueRef = ArrayRef::New(ecmaVm, size);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSMutableHandle<TaggedQueue> queue(thread, factory->NewTaggedQueue(0));
    JSMutableHandle<TaggedNode> node(thread, JSTaggedValue::Undefined());
    JSMutableHandle<JSTaggedValue> currentKey(thread, JSTaggedValue::Undefined());
    Local<JSValueRef> jsValue = StringRef::NewFromUtf8(ecmaVm, "value");
    uint32_t pos = 0;
    uint32_t index = 0;
    while (index < length) {
        node.Update(TaggedHashArray::GetCurrentNode(thread, queue, table, index));
        if (!node.GetTaggedValue().IsHole()) {
            currentKey.Update(node->GetKey());
            if (currentKey->IsECMAObject()) {
                Local<ObjectRef> objRef = ObjectRef::New(ecmaVm);
                objRef->Set(ecmaVm, jsValue, JSNApiHelper::ToLocal<JSValueRef>(currentKey));
                AddInternalProperties(ecmaVm, objRef, ArkInternalValueType::Entry, internalObjects);
                ArrayRef::SetValueAt(ecmaVm, jsValueRef, pos++, objRef);
            } else {
                ArrayRef::SetValueAt(ecmaVm, jsValueRef, pos++, JSNApiHelper::ToLocal<JSValueRef>(currentKey));
            }
        }
    }
    AddInternalProperties(ecmaVm, jsValueRef, ArkInternalValueType::Entry, internalObjects);
    return jsValueRef;
}

Local<JSValueRef> DebuggerApi::GetLightWeightMapValue(const EcmaVM *ecmaVm, Local<JSValueRef> value,
                                                      Global<MapRef> internalObjects)
{
    JSHandle<JSAPILightWeightMap> lightweightMap(JSNApiHelper::ToJSHandle(value));
    uint32_t size  = static_cast<uint32_t>(lightweightMap->GetSize());
    Local<JSValueRef> jsValueRef = ArrayRef::New(ecmaVm, size);
    JSThread *thread = ecmaVm->GetJSThread();
    JSMutableHandle<TaggedArray> keys(thread, lightweightMap->GetKeys());
    JSMutableHandle<TaggedArray> values(thread, lightweightMap->GetValues());
    JSMutableHandle<JSTaggedValue> currentKey(thread, JSTaggedValue::Undefined());
    JSMutableHandle<JSTaggedValue> currentValue(thread, JSTaggedValue::Undefined());
    Local<JSValueRef> jsKey = StringRef::NewFromUtf8(ecmaVm, "key");
    Local<JSValueRef> jsValue = StringRef::NewFromUtf8(ecmaVm, "value");
    uint32_t index = 0;
    while (index < size) {
        currentKey.Update(keys->Get(index));
        currentValue.Update(values->Get(index));
        Local<ObjectRef> objRef = ObjectRef::New(ecmaVm);
        objRef->Set(ecmaVm, jsKey, JSNApiHelper::ToLocal<JSValueRef>(currentKey));
        objRef->Set(ecmaVm, jsValue, JSNApiHelper::ToLocal<JSValueRef>(currentValue));
        AddInternalProperties(ecmaVm, objRef, ArkInternalValueType::Entry, internalObjects);
        ArrayRef::SetValueAt(ecmaVm, jsValueRef, index++, objRef);
    }
    AddInternalProperties(ecmaVm, jsValueRef, ArkInternalValueType::Entry, internalObjects);
    return jsValueRef;
}

Local<JSValueRef> DebuggerApi::GetLightWeightSetValue(const EcmaVM *ecmaVm, Local<JSValueRef> value,
                                                      Global<MapRef> internalObjects)
{
    JSHandle<JSAPILightWeightSet> lightWeightSet(JSNApiHelper::ToJSHandle(value));
    uint32_t size = static_cast<uint32_t>(lightWeightSet->GetSize());
    Local<JSValueRef> jsValueRef = ArrayRef::New(ecmaVm, size);
    JSThread *thread = ecmaVm->GetJSThread();
    JSMutableHandle<JSTaggedValue> currentValue(thread, JSTaggedValue::Undefined());
    Local<JSValueRef> jsValue = StringRef::NewFromUtf8(ecmaVm, "value");
    uint32_t index = 0;
    while (index < size) {
        currentValue.Update(lightWeightSet->GetValueAt(index));
        if (currentValue->IsECMAObject()) {
            Local<ObjectRef> objRef = ObjectRef::New(ecmaVm);
            objRef->Set(ecmaVm, jsValue, JSNApiHelper::ToLocal<JSValueRef>(currentValue));
            AddInternalProperties(ecmaVm, objRef, ArkInternalValueType::Entry, internalObjects);
            ArrayRef::SetValueAt(ecmaVm, jsValueRef, index++, objRef);
        } else {
            ArrayRef::SetValueAt(ecmaVm, jsValueRef, index++, JSNApiHelper::ToLocal<JSValueRef>(currentValue));
        }
    }
    AddInternalProperties(ecmaVm, jsValueRef, ArkInternalValueType::Entry, internalObjects);
    return jsValueRef;
}

Local<JSValueRef> DebuggerApi::GetLinkedListValue(const EcmaVM *ecmaVm, Local<JSValueRef> value,
                                                  Global<MapRef> internalObjects)
{
    JSHandle<JSAPILinkedList> linkedList(JSNApiHelper::ToJSHandle(value));
    JSThread *thread = ecmaVm->GetJSThread();
    JSHandle<TaggedDoubleList> doubleList(thread, linkedList->GetDoubleList());
    uint32_t size = static_cast<uint32_t>(linkedList->Length());
    Local<JSValueRef> jsValueRef = ArrayRef::New(ecmaVm, size);
    JSMutableHandle<JSTaggedValue> currentValue(thread, JSTaggedValue::Undefined());
    int valueNode = TaggedDoubleList::ELEMENTS_START_INDEX;
    uint32_t index = 0;
    while (index < size) {
        valueNode = doubleList->GetNextDataIndex(valueNode);
        currentValue.Update(doubleList->GetElement(valueNode));
        ArrayRef::SetValueAt(ecmaVm, jsValueRef, index++, JSNApiHelper::ToLocal<JSValueRef>(currentValue));
    }
    AddInternalProperties(ecmaVm, jsValueRef, ArkInternalValueType::Entry, internalObjects);
    return jsValueRef;
}

Local<JSValueRef> DebuggerApi::GetListValue(const EcmaVM *ecmaVm, Local<JSValueRef> value,
                                            Global<MapRef> internalObjects)
{
    JSHandle<JSAPIList> list(JSNApiHelper::ToJSHandle(value));
    JSThread *thread = ecmaVm->GetJSThread();
    JSHandle<TaggedSingleList> singleList(thread, list->GetSingleList());
    uint32_t size = static_cast<uint32_t>(list->Length());
    Local<JSValueRef> jsValueRef = ArrayRef::New(ecmaVm, size);
    JSMutableHandle<JSTaggedValue> currentValue(thread, JSTaggedValue::Undefined());
    int valueNode = TaggedDoubleList::ELEMENTS_START_INDEX;
    uint32_t index = 0;
    while (index < size) {
        valueNode = singleList->GetNextDataIndex(valueNode);
        currentValue.Update(singleList->GetElement(valueNode));
        ArrayRef::SetValueAt(ecmaVm, jsValueRef, index++, JSNApiHelper::ToLocal<JSValueRef>(currentValue));
    }
    AddInternalProperties(ecmaVm, jsValueRef, ArkInternalValueType::Entry, internalObjects);
    return jsValueRef;
}

Local<JSValueRef> DebuggerApi::GetPlainArrayValue(const EcmaVM *ecmaVm, Local<JSValueRef> value,
                                                  Global<MapRef> internalObjects)
{
    JSHandle<JSAPIPlainArray> plainarray(JSNApiHelper::ToJSHandle(value));
    uint32_t size = static_cast<uint32_t>(plainarray->GetSize());
    Local<JSValueRef> jsValueRef = ArrayRef::New(ecmaVm, size);
    JSThread *thread = ecmaVm->GetJSThread();
    JSHandle<TaggedArray> keyArray(thread, plainarray->GetKeys());
    JSHandle<TaggedArray> valueArray(thread, plainarray->GetValues());
    JSMutableHandle<JSTaggedValue> currentKey(thread, JSTaggedValue::Undefined());
    JSMutableHandle<JSTaggedValue> currentValue(thread, JSTaggedValue::Undefined());
    Local<JSValueRef> jsKey = StringRef::NewFromUtf8(ecmaVm, "key");
    Local<JSValueRef> jsValue = StringRef::NewFromUtf8(ecmaVm, "value");
    uint32_t index = 0;
    while (index < size) {
        currentKey.Update(keyArray->Get(index));
        currentValue.Update(valueArray->Get(index));
        Local<ObjectRef> objRef = ObjectRef::New(ecmaVm);
        objRef->Set(ecmaVm, jsKey, JSNApiHelper::ToLocal<JSValueRef>(currentKey));
        objRef->Set(ecmaVm, jsValue, JSNApiHelper::ToLocal<JSValueRef>(currentValue));
        AddInternalProperties(ecmaVm, objRef, ArkInternalValueType::Entry, internalObjects);
        ArrayRef::SetValueAt(ecmaVm, jsValueRef, index++, objRef);
    }
    AddInternalProperties(ecmaVm, jsValueRef, ArkInternalValueType::Entry, internalObjects);
    return jsValueRef;
}

Local<JSValueRef> DebuggerApi::GetQueueValue(const EcmaVM *ecmaVm, Local<JSValueRef> value,
                                             Global<MapRef> internalObjects)
{
    JSHandle<JSAPIQueue> queue(JSNApiHelper::ToJSHandle(value));
    uint32_t size = static_cast<uint32_t>(queue->GetSize());
    Local<JSValueRef> jsValueRef = ArrayRef::New(ecmaVm, size);
    JSThread *thread = ecmaVm->GetJSThread();
    JSMutableHandle<JSTaggedValue> currentValue(thread, JSTaggedValue::Undefined());
    uint32_t pos = 0;
    uint32_t index = 0;
    while (index < size) {
        currentValue.Update(queue->Get(thread, pos));
        pos = queue->GetNextPosition(pos);
        ArrayRef::SetValueAt(ecmaVm, jsValueRef, index++, JSNApiHelper::ToLocal<JSValueRef>(currentValue));
    }
    AddInternalProperties(ecmaVm, jsValueRef, ArkInternalValueType::Entry, internalObjects);
    return jsValueRef;
}

Local<JSValueRef> DebuggerApi::GetStackValue(const EcmaVM *ecmaVm, Local<JSValueRef> value,
                                             Global<MapRef> internalObjects)
{
    JSHandle<JSAPIStack> stack(JSNApiHelper::ToJSHandle(value));
    uint32_t size = static_cast<uint32_t>(stack->GetSize());
    Local<JSValueRef> jsValueRef = ArrayRef::New(ecmaVm, size);
    JSThread *thread = ecmaVm->GetJSThread();
    JSMutableHandle<JSTaggedValue> currentValue(thread, JSTaggedValue::Undefined());
    uint32_t index = 0;
    while (index < size + 1) {
        currentValue.Update(stack->Get(index));
        ArrayRef::SetValueAt(ecmaVm, jsValueRef, index++, JSNApiHelper::ToLocal<JSValueRef>(currentValue));
    }
    AddInternalProperties(ecmaVm, jsValueRef, ArkInternalValueType::Entry, internalObjects);
    return jsValueRef;
}

Local<JSValueRef> DebuggerApi::GetTreeMapValue(const EcmaVM *ecmaVm, Local<JSValueRef> value,
                                               Global<MapRef> internalObjects)
{
    JSHandle<JSAPITreeMap> treeMap(JSNApiHelper::ToJSHandle(value));
    uint32_t size = static_cast<uint32_t>(treeMap->GetSize());
    Local<JSValueRef> jsValueRef = ArrayRef::New(ecmaVm, size);
    JSThread *thread = ecmaVm->GetJSThread();
    JSMutableHandle<TaggedTreeMap> iteratedMap(thread, treeMap->GetTreeMap());
    uint32_t elements = static_cast<uint32_t>(iteratedMap->NumberOfElements());
    JSHandle<TaggedArray> entries = TaggedTreeMap::GetArrayFromMap(thread, iteratedMap);
    JSMutableHandle<JSTaggedValue> currentKey(thread, JSTaggedValue::Undefined());
    JSMutableHandle<JSTaggedValue> currentValue(thread, JSTaggedValue::Undefined());
    Local<JSValueRef> jsKey = StringRef::NewFromUtf8(ecmaVm, "key");
    Local<JSValueRef> jsValue = StringRef::NewFromUtf8(ecmaVm, "value");
    uint32_t index = 0;
    while (index < elements) {
        int entriesIndex = entries->Get(index).GetInt();
        currentKey.Update(iteratedMap->GetKey(entriesIndex));
        currentValue.Update(iteratedMap->GetValue(entriesIndex));
        Local<ObjectRef> objRef = ObjectRef::New(ecmaVm);
        objRef->Set(ecmaVm, jsKey, JSNApiHelper::ToLocal<JSValueRef>(currentKey));
        objRef->Set(ecmaVm, jsValue, JSNApiHelper::ToLocal<JSValueRef>(currentValue));
        AddInternalProperties(ecmaVm, objRef, ArkInternalValueType::Entry, internalObjects);
        ArrayRef::SetValueAt(ecmaVm, jsValueRef, index++, objRef);
    }
    AddInternalProperties(ecmaVm, jsValueRef, ArkInternalValueType::Entry, internalObjects);
    return jsValueRef;
}

Local<JSValueRef> DebuggerApi::GetTreeSetValue(const EcmaVM *ecmaVm, Local<JSValueRef> value,
                                               Global<MapRef> internalObjects)
{
    JSHandle<JSAPITreeSet> treeSet(JSNApiHelper::ToJSHandle(value));
    JSThread *thread = ecmaVm->GetJSThread();
    JSMutableHandle<TaggedTreeSet> iteratedSet(thread, treeSet->GetTreeSet());
    uint32_t elements = static_cast<uint32_t>(iteratedSet->NumberOfElements());
    Local<JSValueRef> jsValueRef = ArrayRef::New(ecmaVm, elements);
    JSHandle<TaggedArray> entries = TaggedTreeSet::GetArrayFromSet(thread, iteratedSet);
    JSMutableHandle<JSTaggedValue> currentValue(thread, JSTaggedValue::Undefined());
    Local<JSValueRef> jsValue = StringRef::NewFromUtf8(ecmaVm, "value");
    uint32_t index = 0;
    while (index < elements) {
        int entriesIndex = entries->Get(index).GetInt();
        currentValue.Update(iteratedSet->GetKey(entriesIndex));
        if (currentValue->IsECMAObject()) {
            Local<ObjectRef> objRef = ObjectRef::New(ecmaVm);
            objRef->Set(ecmaVm, jsValue, JSNApiHelper::ToLocal<JSValueRef>(currentValue));
            AddInternalProperties(ecmaVm, objRef, ArkInternalValueType::Entry, internalObjects);
            ArrayRef::SetValueAt(ecmaVm, jsValueRef, index++, objRef);
        } else {
            ArrayRef::SetValueAt(ecmaVm, jsValueRef, index++, JSNApiHelper::ToLocal<JSValueRef>(currentValue));
        }
    }
    AddInternalProperties(ecmaVm, jsValueRef, ArkInternalValueType::Entry, internalObjects);
    return jsValueRef;
}

Local<JSValueRef> DebuggerApi::GetVectorValue(const EcmaVM *ecmaVm, Local<JSValueRef> value,
                                              Global<MapRef> internalObjects)
{
    JSHandle<JSAPIVector> vector(JSNApiHelper::ToJSHandle(value));
    uint32_t size = static_cast<uint32_t>(vector->GetSize());
    Local<JSValueRef> jsValueRef = ArrayRef::New(ecmaVm, size);
    JSThread *thread = ecmaVm->GetJSThread();
    JSMutableHandle<JSTaggedValue> currentValue(thread, JSTaggedValue::Undefined());
    uint32_t index = 0;
    while (index < size) {
        currentValue.Update(vector->Get(thread, vector, index));
        ArrayRef::SetValueAt(ecmaVm, jsValueRef, index++, JSNApiHelper::ToLocal<JSValueRef>(currentValue));
    }
    AddInternalProperties(ecmaVm, jsValueRef, ArkInternalValueType::Entry, internalObjects);
    return jsValueRef;
}
}  // namespace panda::ecmascript::tooling
