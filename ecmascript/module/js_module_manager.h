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

#ifndef ECMASCRIPT_MODULE_JS_MODULE_MANAGER_H
#define ECMASCRIPT_MODULE_JS_MODULE_MANAGER_H

#include "ecmascript/js_tagged_value-inl.h"
#include "ecmascript/jspandafile/js_pandafile.h"

namespace panda::ecmascript {
class ModuleManager {
public:
    explicit ModuleManager(EcmaVM *vm);
    ~ModuleManager() = default;

    JSTaggedValue GetModuleValueInner(JSTaggedValue key);
    JSTaggedValue GetModuleValueInner(JSTaggedValue key, JSTaggedValue jsFunc);
    JSTaggedValue GetModuleValueOutter(JSTaggedValue key);
    JSTaggedValue GetModuleValueOutter(JSTaggedValue key, JSTaggedValue jsFunc);
    void StoreModuleValue(JSTaggedValue key, JSTaggedValue value);
    void StoreModuleValue(JSTaggedValue key, JSTaggedValue value, JSTaggedValue jsFunc);
    JSHandle<SourceTextModule> HostGetImportedModule(const CString &referencingModule);
    JSHandle<SourceTextModule> PUBLIC_API HostResolveImportedModule(const CString &referencingModule);
    JSTaggedValue GetModuleNamespace(JSTaggedValue localName);
    JSTaggedValue GetModuleNamespace(JSTaggedValue localName, JSTaggedValue currentFunc);
    void AddResolveImportedModule(const JSPandaFile *jsPandaFile, const CString &referencingModule);
    void Iterate(const RootVisitor &v);

private:
    NO_COPY_SEMANTIC(ModuleManager);
    NO_MOVE_SEMANTIC(ModuleManager);

    JSTaggedValue GetCurrentModule();
    JSTaggedValue GetModuleValueOutterInternal(JSTaggedValue key, JSTaggedValue currentModule);
    void StoreModuleValueInternal(JSHandle<SourceTextModule> &currentModule,
                                  JSTaggedValue key, JSTaggedValue value);
    JSTaggedValue GetModuleNamespaceInternal(JSTaggedValue localName, JSTaggedValue currentModule);

    static constexpr uint32_t DEAULT_DICTIONART_CAPACITY = 4;

    EcmaVM *vm_ {nullptr};
    JSTaggedValue resolvedModules_ {JSTaggedValue::Hole()};

    friend class EcmaVM;
};
} // namespace panda::ecmascript
#endif // ECMASCRIPT_MODULE_JS_MODULE_MANAGER_H