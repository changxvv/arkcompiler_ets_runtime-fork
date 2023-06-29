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

#ifndef ECMASCRIPT_COMPILER_LLVM_CODEGEN_H
#define ECMASCRIPT_COMPILER_LLVM_CODEGEN_H

#include "ecmascript/compiler/binary_section.h"
#include "ecmascript/compiler/code_generator.h"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include "llvm-c/Core.h"
#include "llvm-c/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JITEventListener.h"

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace panda::ecmascript::kungfu {
class CompilerLog;
class MethodLogList;
class LLVMModule;

struct CodeInfo {
    using sectionInfo = std::pair<uint8_t *, size_t>;
    CodeInfo();

    ~CodeInfo();

    class CodeSpace {
    public:
        static CodeSpace *GetInstance();

        uint8_t *Alloca(uintptr_t size, bool isReq, size_t alignSize, bool alignFlag = true);

    private:
        CodeSpace();
        ~CodeSpace();

        static constexpr size_t REQUIRED_SECS_LIMIT = (1 << 29);  // 512M
        static constexpr size_t UNREQUIRED_SECS_LIMIT = (1 << 28);  // 256M

        // start point of the buffer reserved for sections required in executing phase
        uint8_t *reqSecs_ {nullptr};
        size_t reqBufPos_ {0};
        // start point of the buffer reserved for sections not required in executing phase
        uint8_t *unreqSecs_ {nullptr};
        size_t unreqBufPos_ {0};
    };

    uint8_t *AllocaInReqSecBuffer(uintptr_t size, size_t alignSize = 0, bool alignFlag = true);

    uint8_t *AllocaInNotReqSecBuffer(uintptr_t size, size_t alignSize = 0);

    uint8_t *AllocaCodeSection(uintptr_t size, const char *sectionName);

    uint8_t *AllocaDataSection(uintptr_t size, const char *sectionName);

    void Reset();

    uint8_t *GetSectionAddr(ElfSecName sec) const;

    size_t GetSectionSize(ElfSecName sec) const;

    std::vector<std::pair<uint8_t *, uintptr_t>> GetCodeInfo() const;

    template <class Callback>
    void IterateSecInfos(const Callback &cb) const
    {
        for (size_t i = 0; i < secInfos_.size(); i++) {
            if (secInfos_[i].second == 0) {
                continue;
            }
            cb(i, secInfos_[i]);
        }
    }

private:
    std::array<sectionInfo, static_cast<int>(ElfSecName::SIZE)> secInfos_;
    std::vector<std::pair<uint8_t *, uintptr_t>> codeInfo_ {}; // info for disasssembler, planed to be deprecated
};

enum class FPFlag : uint32_t {
    ELIM_FP = 0,
    RESERVE_FP = 1
};

struct LOptions {
    uint32_t optLevel : 2; // 2 bits for optimized level 0-4
    uint32_t genFp : 1; // 1 bit for whether to generated frame pointer or not
    uint32_t relocMode : 3; // 3 bits for relocation mode
    // 3: default optLevel, 1: generating fp, 2: PIC mode
    LOptions() : optLevel(3), genFp(static_cast<uint32_t>(FPFlag::RESERVE_FP)), relocMode(2) {};
    LOptions(size_t level, FPFlag flag, size_t relocMode)
        : optLevel(level), genFp(static_cast<uint32_t>(flag)), relocMode(relocMode) {};
};

class LLVMAssembler {
public:
    explicit LLVMAssembler(LLVMModule *lm, LOptions option = LOptions());
    virtual ~LLVMAssembler();
    void Run(const CompilerLog &log);
    const LLVMExecutionEngineRef &GetEngine()
    {
        return engine_;
    }
    void Disassemble(const std::map<uintptr_t, std::string> &addr2name, uint64_t textOffset,
                     const CompilerLog &log, const MethodLogList &logList, std::ostringstream &codeStream) const;
    static void Disassemble(const std::map<uintptr_t, std::string> *addr2name,
                            const std::string& triple, uint8_t *buf, size_t size);
    static int GetFpDeltaPrevFramSp(LLVMValueRef fn, const CompilerLog &log);
    static kungfu::CalleeRegAndOffsetVec GetCalleeReg2Offset(LLVMValueRef fn, const CompilerLog &log);

    uintptr_t GetSectionAddr(ElfSecName sec) const
    {
        return reinterpret_cast<uintptr_t>(codeInfo_.GetSectionAddr(sec));
    }

    uint32_t GetSectionSize(ElfSecName sec) const
    {
        return static_cast<uint32_t>(codeInfo_.GetSectionSize(sec));
    }

    void *GetFuncPtrFromCompiledModule(LLVMValueRef function)
    {
        return LLVMGetPointerToGlobal(engine_, function);
    }

    template <class Callback>
    void IterateSecInfos(const Callback &cb) const
    {
        codeInfo_.IterateSecInfos(cb);
    }

    void SetObjFile(const llvm::object::ObjectFile *obj)
    {
        objFile_ = obj;
    }
private:
    class AOTEventListener : public llvm::JITEventListener {
      public:
        AOTEventListener(LLVMAssembler* as) : as_(as)
        {
        }
        void notifyObjectLoaded([[maybe_unused]] ObjectKey key, const llvm::object::ObjectFile &objFile,
                                [[maybe_unused]] const llvm::RuntimeDyld::LoadedObjectInfo &objInfo)
        {
            as_->SetObjFile(&objFile);
        }
      private:
        LLVMAssembler* GetAssembler() const
        {
            return as_;
        }

        LLVMAssembler* as_ {nullptr};
    };

    void UseRoundTripSectionMemoryManager();
    bool BuildMCJITEngine();
    void BuildAndRunPasses();
    void Initialize(LOptions option);
    static void PrintInstAndStep(uint64_t &pc, uint8_t **byteSp, uintptr_t &numBytes, size_t instSize,
                                 uint64_t textOffset, char *outString, std::ostringstream &codeStream,
                                 bool logFlag = true);
    uint64_t GetTextSectionIndex() const;

    LLVMMCJITCompilerOptions options_ {};
    LLVMModule *llvmModule_ {nullptr};
    LLVMModuleRef module_ {nullptr};
    const llvm::object::ObjectFile* objFile_ {nullptr};
    LLVMExecutionEngineRef engine_ {nullptr};
    AOTEventListener listener_;
    char *error_ {nullptr};
    CodeInfo codeInfo_ {};
};

class LLVMIRGeneratorImpl : public CodeGeneratorImpl {
public:
    LLVMIRGeneratorImpl(LLVMModule *module, bool enableLog)
        : module_(module), enableLog_(enableLog) {}
    ~LLVMIRGeneratorImpl() override = default;
    void GenerateCodeForStub(Circuit *circuit, const ControlFlowGraph &graph, size_t index,
                             const CompilationConfig *cfg) override;
    void GenerateCode(Circuit *circuit, const ControlFlowGraph &graph, const CompilationConfig *cfg,
        const MethodLiteral *methodLiteral, const JSPandaFile *jsPandaFile, const std::string &methodName) override;

    bool IsLogEnabled() const
    {
        return enableLog_;
    }

private:
    LLVMModule *module_;
    bool enableLog_ {false};
};
}  // namespace panda::ecmascript::kungfu
#endif  // ECMASCRIPT_COMPILER_LLVM_CODEGEN_H
