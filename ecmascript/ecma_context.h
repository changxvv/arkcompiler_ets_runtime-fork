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
#ifndef ECMASCRIPT_ECMA_CONTEXT_H
#define ECMASCRIPT_ECMA_CONTEXT_H

#include <optional>

#include "ecmascript/base/config.h"
#include "ecmascript/common.h"
#include "ecmascript/frames.h"
#include "ecmascript/js_handle.h"
// #include "ecmascript/compiler/aot_file/aot_file_manager.h"
#include "ecmascript/js_tagged_value.h"
#include "ecmascript/dfx/vmstat/opt_code_profiler.h"
#include "ecmascript/mem/c_containers.h"
#include "ecmascript/mem/visitor.h"
#include "ecmascript/regexp/regexp_parser_cache.h"
#include "libpandafile/file.h"
#include "ecmascript/waiter_list.h"

namespace panda {
class JSNApi;
namespace panda_file {
class File;
}  // namespace panda_file

namespace ecmascript {
class GlobalEnv;
class ObjectFactory;
class EcmaRuntimeStat;
class RegExpParserCache;
class JSPandaFileManager;
class JSPandaFile;
class EcmaStringTable;
class ConstantPool;
class JSPromise;
class RegExpExecResultCache;
class EcmaHandleScope;
enum class PromiseRejectionEvent : uint8_t;

template<typename T>
class JSHandle;
class JSThread;
class JSFunction;
class EcmaVM;
class ModuleManager;
class TSManager;
class AOTFileManager;
class QuickFixManager;
class OptCodeProfiler;

namespace job {
class MicroJobQueue;
}  // namespace job
namespace tooling {
class JsDebuggerManager;
}  // namespace tooling

enum class IcuFormatterType {
    SIMPLE_DATE_FORMAT_DEFAULT,
    SIMPLE_DATE_FORMAT_DATE,
    SIMPLE_DATE_FORMAT_TIME,
    NUMBER_FORMATTER,
    COLLATOR
};

using HostPromiseRejectionTracker = void (*)(const EcmaVM* vm,
                                             const JSHandle<JSPromise> promise,
                                             const JSHandle<JSTaggedValue> reason,
                                             PromiseRejectionEvent operation,
                                             void* data);
using PromiseRejectCallback = void (*)(void* info);
using IcuDeleteEntry = void(*)(void *pointer, void *data);
class EcmaContext {
public:
    static EcmaContext *Create(JSThread *thread);

    static bool Destroy(EcmaContext *context);

    EcmaContext(JSThread *thread);
    ~EcmaContext();

    void SetLoop(void *loop)
    {
        loop_ = loop;
    }

    void *GetLoop() const
    {
        return loop_;
    }
    EcmaVM *GetEcmaVM() const
    {
        return vm_;
    }

    bool Initialize();

    bool ExecutePromisePendingJob();

    static EcmaContext *ConstCast(const EcmaContext *context)
    {
        return const_cast<EcmaContext *>(context);
    }

    EcmaStringTable *GetEcmaStringTable() const
    {
        ASSERT(stringTable_ != nullptr);
        return stringTable_;
    }

    ModuleManager *GetModuleManager() const
    {
        return moduleManager_;
    }

    TSManager *GetTSManager() const
    {
        return tsManager_;
    }

    ARK_INLINE JSThread *GetJSThread() const
    {
        return thread_;
    }
    PromiseRejectCallback GetPromiseRejectCallback() const
    {
        return promiseRejectCallback_;
    }

    void SetPromiseRejectCallback(PromiseRejectCallback cb)
    {
        promiseRejectCallback_ = cb;
    }

    void SetData(void* data)
    {
        data_ = data;
    }

    void PromiseRejectionTracker(const JSHandle<JSPromise> &promise,
                                 const JSHandle<JSTaggedValue> &reason, PromiseRejectionEvent operation)
    {
        if (hostPromiseRejectionTracker_ != nullptr) {
            hostPromiseRejectionTracker_(vm_, promise, reason, operation, data_);
        }
    }

    void SetHostPromiseRejectionTracker(HostPromiseRejectionTracker cb)
    {
        hostPromiseRejectionTracker_ = cb;
    }
    void SetupRegExpResultCache();
    JSHandle<JSTaggedValue> GetRegExpCache() const
    {
        return JSHandle<JSTaggedValue>(reinterpret_cast<uintptr_t>(&regexpCache_));
    }

    RegExpParserCache *GetRegExpParserCache() const
    {
        ASSERT(regExpParserCache_ != nullptr);
        return regExpParserCache_;
    }

    void SetRegExpCache(JSTaggedValue newCache)
    {
        regexpCache_ = newCache;
    }

    WaiterListNode *GetWaiterListNode()
    {
        return &waiterListNode_;
    }

    void SetAllowAtomicWait(bool wait)
    {
        AllowAtomicWait_ = wait;
    }

    bool GetAllowAtomicWait() const
    {
        return AllowAtomicWait_;
    }
    JSHandle<ecmascript::JSTaggedValue> GetAndClearEcmaUncaughtException() const;
    JSHandle<ecmascript::JSTaggedValue> GetEcmaUncaughtException() const;
    void EnableUserUncaughtErrorHandler();

    void AddConstpool(const JSPandaFile *jsPandaFile, JSTaggedValue constpool, int32_t index = 0);

    bool HasCachedConstpool(const JSPandaFile *jsPandaFile) const;

    JSTaggedValue FindConstpool(const JSPandaFile *jsPandaFile, int32_t index);
    // For new version instruction.
    JSTaggedValue FindConstpool(const JSPandaFile *jsPandaFile, panda_file::File::EntityId id);
    std::optional<std::reference_wrapper<CMap<int32_t, JSTaggedValue>>> FindConstpools(
        const JSPandaFile *jsPandaFile);

    JSHandle<ConstantPool> PUBLIC_API FindOrCreateConstPool(const JSPandaFile *jsPandaFile,
                                                            panda_file::File::EntityId id);
    void CreateAllConstpool(const JSPandaFile *jsPandaFile);

    void HandleUncaughtException(JSTaggedValue exception);
    void ProcessNativeDelete(const WeakRootVisitor &visitor);
    void ProcessReferences(const WeakRootVisitor &visitor);
    JSHandle<GlobalEnv> GetGlobalEnv() const;
    JSHandle<job::MicroJobQueue> GetMicroJobQueue() const;

    void PrintJSErrorInfo(const JSHandle<JSTaggedValue> &exceptionInfo);
    void Iterate(const RootVisitor &v, const RootRangeVisitor &rv);
    static void MountContext(JSThread *thread);
    static void UnmountContext(JSThread *thread);
    void SetMicroJobQueue(job::MicroJobQueue *queue);
    void SetGlobalEnv(GlobalEnv *global);

    OptCodeProfiler *GetOptCodeProfiler() const
    {
        return optCodeProfiler_;
    }

    // For icu objects cache
    void SetIcuFormatterToCache(IcuFormatterType type, const std::string &locale, void *icuObj,
                                IcuDeleteEntry deleteEntry = nullptr)
    {
        EcmaContext::IcuFormatter icuFormatter = IcuFormatter(locale, icuObj, deleteEntry);
        icuObjCache_.insert_or_assign(type, std::move(icuFormatter));
    }

    void *GetIcuFormatterFromCache(IcuFormatterType type, std::string locale)
    {
        auto iter = icuObjCache_.find(type);
        if (iter != icuObjCache_.end()) {
            EcmaContext::IcuFormatter icuFormatter = iter->second;
            if (icuFormatter.locale == locale) {
                return icuFormatter.icuObj;
            }
        }
        return nullptr;
    }

    void ClearIcuCache()
    {
        auto iter = icuObjCache_.begin();
        while (iter != icuObjCache_.end()) {
            EcmaContext::IcuFormatter icuFormatter = iter->second;
            IcuDeleteEntry deleteEntry = icuFormatter.deleteEntry;
            if (deleteEntry != nullptr) {
                deleteEntry(icuFormatter.icuObj, vm_);
            }
            iter->second = EcmaContext::IcuFormatter{};
            iter++;
        }
    }
    AOTFileManager *GetAOTFileManager() const
    {
        return aotFileManager_;
    }
    EcmaRuntimeStat *GetRuntimeStat() const
    {
        return runtimeStat_;
    }
    void SetRuntimeStatEnable(bool flag);
    void InitializeEcmaScriptRunStat();
    void DumpAOTInfo() const DUMP_API_ATTR;

    JSTaggedValue ExecuteAot(size_t actualNumArgs, JSTaggedType *args, const JSTaggedType *prevFp,
                             OptimizedEntryFrame::CallType callType);
    void LoadStubFile();
    void ClearBufferData();
private:
    JSTaggedValue InvokeEcmaAotEntrypoint(JSHandle<JSFunction> mainFunc, JSHandle<JSTaggedValue> &thisArg,
                                          const JSPandaFile *jsPandaFile, std::string_view entryPoint);    
    Expected<JSTaggedValue, bool> InvokeEcmaEntrypoint(const JSPandaFile *jsPandaFile, std::string_view entryPoint,
                                                       bool excuteFromJob = false);
    bool LoadAOTFiles(const std::string& aotFileName);
    NO_MOVE_SEMANTIC(EcmaContext);
    NO_COPY_SEMANTIC(EcmaContext);

    JSThread *thread_ {nullptr};
    EcmaVM *vm_ {nullptr};
    ObjectFactory *factory_ {nullptr};
    ModuleManager *moduleManager_ {nullptr};

    bool isUncaughtExceptionRegistered_ {false};
    bool isProcessingPendingJob_ {false};

    EcmaStringTable *stringTable_ {nullptr};
    JSTaggedValue globalEnv_ {JSTaggedValue::Hole()};

    JSTaggedValue microJobQueue_ {JSTaggedValue::Hole()};

    EcmaRuntimeStat *runtimeStat_ {nullptr};
    // VM execution states.
    RegExpParserCache *regExpParserCache_ {nullptr};
    JSTaggedValue regexpCache_ {JSTaggedValue::Hole()};
    CMap<const JSPandaFile *, CMap<int32_t, JSTaggedValue>> cachedConstpools_ {};

    AOTFileManager *aotFileManager_ {nullptr};
    // VM resources.
    TSManager *tsManager_ {nullptr};

    // atomics
    bool AllowAtomicWait_ {true};
    WaiterListNode waiterListNode_;

    // Registered Callbacks
    PromiseRejectCallback promiseRejectCallback_ {nullptr};
    HostPromiseRejectionTracker hostPromiseRejectionTracker_ {nullptr};
    void* data_ {nullptr};

    // opt code Profiler
    OptCodeProfiler *optCodeProfiler_ {nullptr};

    // For icu objects cache
    struct IcuFormatter {
        std::string locale;
        void *icuObj {nullptr};
        IcuDeleteEntry deleteEntry {nullptr};

        IcuFormatter() = default;
        IcuFormatter(const std::string &locale, void *icuObj, IcuDeleteEntry deleteEntry = nullptr)
            : locale(locale), icuObj(icuObj), deleteEntry(deleteEntry) {}
    };
    std::unordered_map<IcuFormatterType, IcuFormatter> icuObjCache_;
    void *loop_ {nullptr};

    static const uint32_t NODE_BLOCK_SIZE_LOG2 = 10;
    static const uint32_t NODE_BLOCK_SIZE = 1U << NODE_BLOCK_SIZE_LOG2;
    static constexpr int32_t MIN_HANDLE_STORAGE_SIZE = 2;
    JSTaggedType *handleScopeStorageNext_ {nullptr};
    JSTaggedType *handleScopeStorageEnd_ {nullptr};
    std::vector<std::array<JSTaggedType, NODE_BLOCK_SIZE> *> handleStorageNodes_ {};
    int32_t currentHandleStorageIndex_ {-1};
    int32_t handleScopeCount_ {0};
    EcmaHandleScope *lastHandleScope_ {nullptr};
    JSTaggedType *currentFrame_ {nullptr};
    JSTaggedType *leaveFrame_ {nullptr};
    JSTaggedType *lastFp_ {nullptr};
    PropertiesCache *propertiesCache_ {nullptr};

    friend class EcmaHandleScope;
    friend class JSPandaFileExecutor;
    friend class ObjectFactory;
    friend class panda::JSNApi;
    friend class AOTFileManager;
};
}  // namespace ecmascript
}  // namespace panda
#endif // ECMASCRIPT_ECMA_CONTEXT_H
