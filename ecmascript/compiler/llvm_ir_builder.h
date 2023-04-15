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

#ifndef ECMASCRIPT_COMPILER_LLVM_IR_BUILDER_H
#define ECMASCRIPT_COMPILER_LLVM_IR_BUILDER_H

#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#include "ecmascript/compiler/circuit.h"
#include "ecmascript/compiler/gate.h"
#include "ecmascript/compiler/stub_builder.h"
#include "ecmascript/compiler/call_signature.h"
#include "ecmascript/compiler/common_stubs.h"
#include "ecmascript/compiler/interpreter_stub.h"
#include "ecmascript/compiler/rt_call_signature.h"
#include "ecmascript/jspandafile/method_literal.h"
#include "llvm-c/DebugInfo.h"
#include "llvm-c/Core.h"

namespace panda::ecmascript::kungfu {
using OperandsVector = std::set<int>;
class BasicBlock;
class DebugInfo;
using BasicBlockMap = std::map<int, std::unique_ptr<BasicBlock>>;
class LLVMIRBuilder;
using HandleType = void(LLVMIRBuilder::*)(GateRef gate);

enum class MachineRep {
    K_NONE,
    K_BIT,
    K_WORD8,
    K_WORD16,
    K_WORD32,
    K_WORD64,
    // FP representations must be last, and in order of increasing size.
    K_FLOAT32,
    K_FLOAT64,
    K_SIMD128,
    K_PTR_1, // Tagged Pointer
    K_META,
};

class BasicBlock {
public:
    explicit BasicBlock(int id) : id_(id)
    {
        predecessors_ = {};
        successors_ = {};
        impl_ = nullptr;
    }

    int GetId() const
    {
        return id_;
    }

    template<class T>
    inline T *GetImpl() const
    {
        return static_cast<T *>(impl_);
    }

    inline void SetImpl(void *impl)
    {
        impl_ = impl;
    }

    template<class T>
    inline void ResetImpl()
    {
        if (impl_) {
            delete GetImpl<T>();
            impl_ = nullptr;
        }
    }
    ~BasicBlock() = default;

private:
    std::vector<BasicBlock *> predecessors_ {};
    std::vector<BasicBlock *> successors_ {};
    int id_ {-1};
    void *impl_ {nullptr};
};

struct NotMergedPhiDesc {
    BasicBlock *pred;
    GateRef operand;
    LLVMValueRef phi;
};

struct BasicBlockImpl {
    LLVMBasicBlockRef lBB_ = nullptr;
    LLVMBasicBlockRef continuation = nullptr;
    bool started = false;
    bool ended = false;
    std::vector<NotMergedPhiDesc> unmergedPhis_;
};

class LLVMModule {
public:
    LLVMModule(NativeAreaAllocator* allocator, const std::string &name, bool logDbg, const std::string &triple);
    ~LLVMModule();
    void SetUpForCommonStubs();
    void SetUpForBytecodeHandlerStubs();
    void SetUpForBuiltinsStubs();
    LLVMValueRef AddFunc(const panda::ecmascript::MethodLiteral *methodLiteral, const JSPandaFile *jsPandaFile);
    LLVMModuleRef GetModule() const
    {
        return module_;
    }
    LLVMTypeRef GetFuncType(const CallSignature *stubDescriptor);

    LLVMTypeRef GenerateFuncType(const std::vector<LLVMValueRef> &params, const CallSignature *stubDescriptor);

    void SetFunction(size_t index, LLVMValueRef func)
    {
        funcIndexMap_.emplace_back(std::make_pair(index, func));
    }

    LLVMValueRef GetFunction(size_t index)
    {
        // next optimization can be performed
        for (auto &it: funcIndexMap_) {
            if (it.first == index) {
                return it.second;
            }
        }
        return nullptr;
    }

    bool Is64Bit() const
    {
        return is64Bit_;
    }

    bool Is32Bit() const
    {
        return !is64Bit_;
    }

    size_t GetFuncCount() const
    {
        return funcIndexMap_.size();
    }

    template<class Callback>
    void IteratefuncIndexMap(const Callback &cb) const
    {
        for (auto record : funcIndexMap_) {
            cb(record.first, record.second);
        }
    }

    const CallSignature *GetCSign(size_t index) const
    {
        return callSigns_[index];
    }

    const std::string &GetTripleStr() const
    {
        return tripleStr_;
    }

    const std::vector<const CallSignature*> &GetCSigns() const
    {
        return callSigns_;
    }

    LLVMMetadataRef GetDFileMD() const
    {
        return dFileMD_;
    }

    LLVMDIBuilderRef GetDIBuilder() const
    {
        return dBuilder_;
    }

    LLVMValueRef GetDeoptFunction();

    DebugInfo* GetDebugInfo() const
    {
        return debugInfo_;
    }

    Triple GetTriple() const
    {
        return triple_;
    }

    std::string GetFuncName(const panda::ecmascript::MethodLiteral *methodLiteral, const JSPandaFile *jsPandaFile);

    static constexpr int kDeoptEntryOffset = 0;

private:
    LLVMValueRef AddAndGetFunc(const CallSignature *stubDescriptor);
    void InitialLLVMFuncTypeAndFuncByModuleCSigns();
    LLVMTypeRef ConvertLLVMTypeFromVariableType(VariableType type);
    LLVMTypeRef NewLType(MachineType machineType, GateType gateType);
    // index:
    //     stub scenario - sequence of function adding to llvmModule
    //     aot scenario - method Id of function generated by panda files
    std::vector<std::pair<size_t, LLVMValueRef>> funcIndexMap_;
    std::vector<const CallSignature *> callSigns_;
    LLVMModuleRef module_ {nullptr};
    LLVMMetadataRef dFileMD_ {nullptr};
    LLVMMetadataRef dUnitMD_ {nullptr};
    LLVMDIBuilderRef dBuilder_ {nullptr};
    DebugInfo* debugInfo_ {nullptr};
    std::string tripleStr_;
    bool is64Bit_ {false};
    Triple triple_;
};


#define OPCODES(V)                                                                        \
    V(Call, (GateRef gate, const std::vector<GateRef> &inList, OpCode op))                \
    V(RuntimeCall, (GateRef gate, const std::vector<GateRef> &inList))                    \
    V(RuntimeCallWithArgv, (GateRef gate, const std::vector<GateRef> &inList))            \
    V(NoGcRuntimeCall, (GateRef gate, const std::vector<GateRef> &inList))                \
    V(BytecodeCall, (GateRef gate, const std::vector<GateRef> &inList))                   \
    V(Alloca, (GateRef gate))                                                             \
    V(Block, (int id, const OperandsVector &predecessors))                                \
    V(Goto, (int block, int bbout))                                                       \
    V(Parameter, (GateRef gate))                                                          \
    V(Constant, (GateRef gate, std::bitset<64> value))                                    \
    V(ConstString, (GateRef gate, const ChunkVector<char> &str))                          \
    V(RelocatableData, (GateRef gate, uint64_t value))                                    \
    V(ZExtInt, (GateRef gate, GateRef e1))                                                \
    V(SExtInt, (GateRef gate, GateRef e1))                                                \
    V(FPExt, (GateRef gate, GateRef e1))                                                  \
    V(FPTrunc, (GateRef gate, GateRef e1))                                                \
    V(Load, (GateRef gate, GateRef base))                                                 \
    V(Store, (GateRef gate, GateRef base, GateRef value))                                 \
    V(IntRev, (GateRef gate, GateRef e1))                                                 \
    V(Add, (GateRef gate, GateRef e1, GateRef e2))                                        \
    V(Sub, (GateRef gate, GateRef e1, GateRef e2))                                        \
    V(Mul, (GateRef gate, GateRef e1, GateRef e2))                                        \
    V(FloatDiv, (GateRef gate, GateRef e1, GateRef e2))                                   \
    V(IntDiv, (GateRef gate, GateRef e1, GateRef e2))                                     \
    V(UDiv, (GateRef gate, GateRef e1, GateRef e2))                                       \
    V(IntOr, (GateRef gate, GateRef e1, GateRef e2))                                      \
    V(IntAnd, (GateRef gate, GateRef e1, GateRef e2))                                     \
    V(IntXor, (GateRef gate, GateRef e1, GateRef e2))                                     \
    V(IntLsr, (GateRef gate, GateRef e1, GateRef e2))                                     \
    V(IntAsr, (GateRef gate, GateRef e1, GateRef e2))                                     \
    V(Int32LessThanOrEqual, (GateRef gate, GateRef e1, GateRef e2))                       \
    V(Cmp, (GateRef gate, GateRef e1, GateRef e2))                                        \
    V(Branch, (GateRef gate, GateRef cmp, GateRef btrue, GateRef bfalse))                 \
    V(Switch, (GateRef gate, GateRef input, const std::vector<GateRef> &outList))         \
    V(SwitchCase, (GateRef gate, GateRef switchBranch, GateRef out))                      \
    V(Phi, (GateRef gate, const std::vector<GateRef> &srcGates))                          \
    V(Return, (GateRef gate, GateRef popCount, const std::vector<GateRef> &operands))     \
    V(ReturnVoid, (GateRef gate))                                                         \
    V(CastIntXToIntY, (GateRef gate, GateRef e1))                                         \
    V(ChangeInt32ToDouble, (GateRef gate, GateRef e1))                                    \
    V(ChangeUInt32ToDouble, (GateRef gate, GateRef e1))                                   \
    V(ChangeDoubleToInt32, (GateRef gate, GateRef e1))                                    \
    V(BitCast, (GateRef gate, GateRef e1))                                                \
    V(IntLsl, (GateRef gate, GateRef e1, GateRef e2))                                     \
    V(Mod, (GateRef gate, GateRef e1, GateRef e2))                                        \
    V(ChangeTaggedPointerToInt64, (GateRef gate, GateRef e1))                             \
    V(ChangeInt64ToTagged, (GateRef gate, GateRef e1))                                    \
    V(Deopt, (GateRef gate))                                                              \
    V(TruncFloatToInt, (GateRef gate, GateRef e1))                                        \
    V(AddWithOverflow, (GateRef gate, GateRef e1, GateRef e2))                            \
    V(SubWithOverflow, (GateRef gate, GateRef e1, GateRef e2))                            \
    V(MulWithOverflow, (GateRef gate, GateRef e1, GateRef e2))                            \
    V(ExtractValue, (GateRef gate, GateRef e1, GateRef e2))                               \
    V(Sqrt, (GateRef gate, GateRef e1))

// runtime/common stub ID, opcodeOffset for bc stub
using StubIdType = std::variant<RuntimeStubCSigns::ID, CommonStubCSigns::ID, LLVMValueRef>;

class LLVMIRBuilder {
public:
    LLVMIRBuilder(const std::vector<std::vector<GateRef>> *schedule, Circuit *circuit,
                  LLVMModule *module, LLVMValueRef function, const CompilationConfig *cfg,
                  CallSignature::CallConv callConv, bool enableLog, const std::string &funcName);
    ~LLVMIRBuilder();
    void Build();

private:
    #define DECLAREVISITOPCODE(name, signature) void Visit##name signature;
        OPCODES(DECLAREVISITOPCODE)
    #undef DECLAREVISITOPCODE
    #define DECLAREHANDLEOPCODE(name, ignore) void Handle##name(GateRef gate);
        OPCODES(DECLAREHANDLEOPCODE)
    #undef DECLAREHANDLEOPCODE

    bool isPrologue(int bbId) const
    {
        return bbId == 0;
    }
    void LinkToLLVMCfg(int bbId, const OperandsVector &predecessors);
    BasicBlock *EnsureBB(int id);
    LLVMValueRef CallingFp(LLVMModuleRef &module, LLVMBuilderRef &builder, bool isCaller);
    LLVMValueRef GetCurrentSP();
    LLVMValueRef ReadRegister(LLVMModuleRef &module, LLVMBuilderRef &builder, LLVMMetadataRef meta);
    void GenPrologue();
    LLVMBasicBlockRef EnsureLBB(BasicBlock *bb) const;
    BasicBlockImpl *EnsureBBImpl(BasicBlock *bb) const;
    void SetToCfg(BasicBlock *bb) const;

    LLVMTypeRef GetMachineRepType(MachineRep rep) const;
    int FindBasicBlock(GateRef gate) const;
    void EndCurrentBlock() const;
    void Finish();

    void ProcessPhiWorkList();
    void InitializeHandlers();
    std::string LLVMValueToString(LLVMValueRef val) const;

    LLVMTypeRef GetIntPtr() const
    {
        if (compCfg_->Is32Bit()) {
            return LLVMInt32Type();
        }
        return LLVMInt64Type();
    }
    LLVMTypeRef ConvertLLVMTypeFromGate(GateRef gate) const;
    int64_t GetBitWidthFromMachineType(MachineType machineType) const;
    LLVMValueRef PointerAdd(LLVMValueRef baseAddr, LLVMValueRef offset, LLVMTypeRef rep);
    LLVMValueRef VectorAdd(LLVMValueRef e1Value, LLVMValueRef e2Value, LLVMTypeRef rep);
    LLVMValueRef CanonicalizeToInt(LLVMValueRef value);
    LLVMValueRef CanonicalizeToPtr(LLVMValueRef value);
    LLVMValueRef GetCurrentFrameType(LLVMValueRef currentSpFrameAddr);
    void SetFunctionCallConv();

    bool IsLogEnabled() const
    {
        return enableLog_;
    }
    LLVMValueRef GetFunction(LLVMValueRef glue, const CallSignature *signature, LLVMValueRef rtbaseoffset,
                             const std::string &realName = "") const;
    LLVMValueRef GetCallee(const std::vector<GateRef> &inList, const CallSignature *signature,
                           const std::string &realName = "");
    LLVMValueRef GetFunctionFromGlobalValue(LLVMValueRef glue, const CallSignature *signature,
                                            LLVMValueRef reloc) const;
    bool IsInterpreted() const;
    bool IsOptimized() const;
    bool IsOptimizedJSFunction() const;
    void SetGCLeafFunction(LLVMValueRef call);
    void SetCallConvAttr(const CallSignature *calleeDescriptor, LLVMValueRef call);
    bool IsHeapPointerType(LLVMTypeRef valueType);

private:
    enum class CallInputs : size_t {
        DEPEND = 0,
        TARGET,
        GLUE,
        FIRST_PARAMETER
    };
    enum class CallExceptionKind : bool {
        HAS_PC_OFFSET = true,
        NO_PC_OFFSET = false
    };

    LLVMDIBuilderRef GetDIBuilder() const
    {
        return llvmModule_ == nullptr ? nullptr : llvmModule_->GetDIBuilder();
    }

    LLVMRealPredicate ConvertLLVMPredicateFromFCMP(FCmpCondition cond);
    LLVMIntPredicate ConvertLLVMPredicateFromICMP(ICmpCondition cond);
    LLVMValueRef GetGlue(const std::vector<GateRef> &inList);
    LLVMValueRef GetLeaveFrameOffset(LLVMValueRef glue);
    LLVMValueRef GetRTStubOffset(LLVMValueRef glue, int index);
    LLVMValueRef GetCoStubOffset(LLVMValueRef glue, int index);
    LLVMValueRef GetBCStubOffset(LLVMValueRef glue);
    LLVMValueRef GetBCDebugStubOffset(LLVMValueRef glue);
    LLVMValueRef GetBuiltinsStubOffset(LLVMValueRef glue);
    LLVMValueRef GetBaseOffset(GateRef gate, LLVMValueRef glue);
    CallExceptionKind GetCallExceptionKind(size_t index, OpCode op) const;
    void ComputeArgCountAndPCOffset(size_t &actualNumArgs, LLVMValueRef &pcOffset,
                                    const std::vector<GateRef> &inList, CallExceptionKind kind);
    void SaveLexicalEnvOnOptJSFuncFrame(LLVMValueRef value);
    void SaveJSFuncOnOptJSFuncFrame(LLVMValueRef value);
    void SaveFrameTypeOnFrame(FrameType frameType, LLVMBuilderRef builder);
    void UpdateLeaveFrame(LLVMValueRef glue);
    LLVMTypeRef GetExperimentalDeoptTy();
    LLVMValueRef GetExperimentalDeopt(LLVMModuleRef &module);
    void GenDeoptEntry(LLVMModuleRef &module);
    LLVMMetadataRef GetFunctionTypeMD(LLVMMetadataRef dFile);
    bool SetDebugInfo(GateRef g, LLVMValueRef r);

    const CompilationConfig *compCfg_ {nullptr};
    const std::vector<std::vector<GateRef>> *scheduledGates_ {nullptr};
    const Circuit *circuit_ {nullptr};
    GateAccessor acc_;
    BasicBlock *currentBb_ {nullptr};
    int lineNumber_ {0};

    LLVMModuleRef module_ {nullptr};
    LLVMContextRef context_ {nullptr};
    LLVMValueRef function_ {nullptr};
    LLVMBuilderRef builder_ {nullptr};
    std::map<GateId, int> instID2bbID_;
    BasicBlockMap bbID2BB_;

    std::vector<BasicBlock *> phiRebuildWorklist_;
    LLVMModule *llvmModule_ {nullptr};
    std::unordered_map<GateRef, LLVMValueRef> gate2LValue_;
    std::unordered_map<OpCode, HandleType> opHandlers_;
    std::set<OpCode> illegalOpHandlers_;
    int slotSize_ {-1};
    LLVMTypeRef slotType_ {nullptr};
    CallSignature::CallConv callConv_ = CallSignature::CallConv::CCallConv;
    bool enableLog_ {false};
    LLVMMetadataRef dFuncMD_ {nullptr};
};
}  // namespace panda::ecmascript::kungfu
#endif  // ECMASCRIPT_COMPILER_LLVM_IR_BUILDER_H
