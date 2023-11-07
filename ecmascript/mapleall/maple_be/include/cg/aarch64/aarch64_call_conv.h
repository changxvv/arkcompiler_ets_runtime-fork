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

#ifndef MAPLEBE_INCLUDE_CG_AARCH64_AARCH64_CALL_CONV_H
#define MAPLEBE_INCLUDE_CG_AARCH64_AARCH64_CALL_CONV_H

#include "types_def.h"
#include "becommon.h"
#include "call_conv.h"
#include "aarch64_abi.h"
#include "abi.h"

namespace maplebe {
using namespace maple;

/*
 * We use the names used in ARM IHI 0055C_beta. $ 5.4.2.
 * nextGeneralRegNO (= _int_parm_num)  : Next General-purpose Register number
 * nextFloatRegNO (= _float_parm_num): Next SIMD and Floating-point Register Number
 * nextStackArgAdress (= _last_memOffset): Next Stacked Argument Address
 * for processing an incoming or outgoing parameter list
 */

class AArch64CallConvImpl : public CCImpl {
public:
    explicit AArch64CallConvImpl(BECommon &be) : CCImpl(), beCommon(be) {}

    ~AArch64CallConvImpl() = default;

    /* Return size of aggregate structure copy on stack. */
    int32 LocateNextParm(MIRType &mirType, CCLocInfo &pLoc, bool isFirst = false, MIRFunction *func = nullptr) override;

    int32 LocateRetVal(MIRType &retType, CCLocInfo &ploc) override;

    /* for lmbc */
    uint32 FloatParamRegRequired(MIRStructType &structType, uint32 &fpSize);

    /*  return value related  */
    void InitReturnInfo(MIRType &retTy, CCLocInfo &pLoc) override;

    void SetupSecondRetReg(const MIRType &retTy2, CCLocInfo &pLoc) const override;

    void SetupToReturnThroughMemory(CCLocInfo &pLoc) const
    {
        pLoc.regCount = 1;
        pLoc.reg0 = R8;
        pLoc.primTypeOfReg0 = PTY_u64;
    }

    void Init() override
    {
        paramNum = 0;
        nextGeneralRegNO = 0;
        nextFloatRegNO = 0;
        nextStackArgAdress = 0;
    }

private:
    BECommon &beCommon;
    uint64 paramNum = 0;        /* number of all types of parameters processed so far */
    int32 nextGeneralRegNO = 0; /* number of integer parameters processed so far */
    uint32 nextFloatRegNO = 0;  /* number of float parameters processed so far */

    AArch64reg AllocateGPRegister()
    {
        DEBUG_ASSERT(nextGeneralRegNO >= 0, "nextGeneralRegNO can not be neg");
        return (nextGeneralRegNO < AArch64Abi::kNumIntParmRegs) ? AArch64Abi::intParmRegs[nextGeneralRegNO++]
                                                                : kRinvalid;
    }

    void AllocateTwoGPRegisters(CCLocInfo &pLoc)
    {
        if ((nextGeneralRegNO + 1) < AArch64Abi::kNumIntParmRegs) {
            pLoc.reg0 = AArch64Abi::intParmRegs[nextGeneralRegNO++];
            pLoc.reg1 = AArch64Abi::intParmRegs[nextGeneralRegNO++];
        } else {
            pLoc.reg0 = kRinvalid;
        }
    }

    AArch64reg AllocateSIMDFPRegister()
    {
        return (nextFloatRegNO < AArch64Abi::kNumFloatParmRegs) ? AArch64Abi::floatParmRegs[nextFloatRegNO++]
                                                                : kRinvalid;
    }

    void AllocateNSIMDFPRegisters(CCLocInfo &ploc, uint32 num)
    {
        if ((nextFloatRegNO + num - 1) < AArch64Abi::kNumFloatParmRegs) {
            switch (num) {
                case kOneRegister:
                    ploc.reg0 = AArch64Abi::floatParmRegs[nextFloatRegNO++];
                    break;
                case kTwoRegister:
                    ploc.reg0 = AArch64Abi::floatParmRegs[nextFloatRegNO++];
                    ploc.reg1 = AArch64Abi::floatParmRegs[nextFloatRegNO++];
                    break;
                case kThreeRegister:
                    ploc.reg0 = AArch64Abi::floatParmRegs[nextFloatRegNO++];
                    ploc.reg1 = AArch64Abi::floatParmRegs[nextFloatRegNO++];
                    ploc.reg2 = AArch64Abi::floatParmRegs[nextFloatRegNO++];
                    break;
                case kFourRegister:
                    ploc.reg0 = AArch64Abi::floatParmRegs[nextFloatRegNO++];
                    ploc.reg1 = AArch64Abi::floatParmRegs[nextFloatRegNO++];
                    ploc.reg2 = AArch64Abi::floatParmRegs[nextFloatRegNO++];
                    ploc.reg3 = AArch64Abi::floatParmRegs[nextFloatRegNO++];
                    break;
                default:
                    CHECK_FATAL(0, "AllocateNSIMDFPRegisters: unsupported");
            }
        } else {
            ploc.reg0 = kRinvalid;
        }
    }

    void RoundNGRNUpToNextEven()
    {
        nextGeneralRegNO = static_cast<int32>((nextGeneralRegNO + 1) & ~static_cast<int32>(1));
    }

    int32 ProcessPtyAggWhenLocateNextParm(MIRType &mirType, CCLocInfo &pLoc, uint64 &typeSize, int32 typeAlign);
};

class AArch64WebKitJSCC : public CCImpl {
public:
    explicit AArch64WebKitJSCC(BECommon &be) : CCImpl(), beCommon(be) {}

    ~AArch64WebKitJSCC() = default;

    /* Return size of aggregate structure copy on stack. */
    int32 LocateNextParm(MIRType &mirType, CCLocInfo &pLoc, bool isFirst = false, MIRFunction *func = nullptr) override;

    int32 LocateRetVal(MIRType &retType, CCLocInfo &ploc) override;

    /*  return value related  */
    void InitReturnInfo(MIRType &retTy, CCLocInfo &pLoc) override;

    // invalid interface
    void SetupSecondRetReg(const MIRType &retTy2, CCLocInfo &pLoc) const override;

    void Init() override
    {
        nextGeneralRegNO = 0;
        nextFloatRegNO = 0;
        nextStackArgAdress = 0;
    }

private:
    BECommon &beCommon;
    int32 nextGeneralRegNO = 0; /* number of integer parameters processed so far */
    uint32 nextFloatRegNO = 0;  /* number of float parameters processed so far */
    static constexpr int32 kNumIntRetRegs = 8;
    static constexpr int32 kNumFloatRetRegs = 8;
    static constexpr int32 kNumIntParmRegs = 1;
    static constexpr int32 kNumFloatParmRegs = 0;
    static constexpr AArch64reg intReturnRegs[kNumIntRetRegs] = {R0, R1, R2, R3, R4, R5, R6, R7};
    static constexpr AArch64reg floatReturnRegs[kNumFloatRetRegs] = {V0, V1, V2, V3, V4, V5, V6, V7};
    static constexpr AArch64reg intParmRegs[kNumIntParmRegs] = {R0};
    static constexpr AArch64reg floatParmRegs[kNumFloatParmRegs] = {};

    int32 ClassificationArg(const BECommon &be, MIRType &mirType, std::vector<ArgumentClass> &classes) const;

    int32 ClassificationRet(const BECommon &be, MIRType &mirType, std::vector<ArgumentClass> &classes) const;

    AArch64reg AllocateGPParmRegister()
    {
        DEBUG_ASSERT(nextGeneralRegNO >= 0, "nextGeneralRegNO can not be neg");
        return (nextGeneralRegNO < AArch64WebKitJSCC::kNumIntParmRegs)
                   ? AArch64WebKitJSCC::intParmRegs[nextGeneralRegNO++]
                   : kRinvalid;
    }

    AArch64reg AllocateGPRetRegister()
    {
        DEBUG_ASSERT(nextGeneralRegNO >= 0, "nextGeneralRegNO can not be neg");
        return (nextGeneralRegNO < AArch64WebKitJSCC::kNumIntRetRegs)
                   ? AArch64WebKitJSCC::intReturnRegs[nextGeneralRegNO++]
                   : kRinvalid;
    }

    AArch64reg AllocateSIMDFPRetRegister()
    {
        return (nextFloatRegNO < AArch64WebKitJSCC::kNumFloatParmRegs)
                   ? AArch64WebKitJSCC::floatReturnRegs[nextFloatRegNO++]
                   : kRinvalid;
    }
};

class GHCCC : public CCImpl {
public:
    explicit GHCCC(BECommon &be) : CCImpl(), beCommon(be) {}

    ~GHCCC() = default;

    /* Return size of aggregate structure copy on stack. */
    int32 LocateNextParm(MIRType &mirType, CCLocInfo &pLoc, bool isFirst = false, MIRFunction *func = nullptr) override;

    int32 LocateRetVal(MIRType &retType, CCLocInfo &ploc) override;

    /*  return value related  */
    void InitReturnInfo(MIRType &retTy, CCLocInfo &pLoc) override;

    // invalid interface
    void SetupSecondRetReg(const MIRType &retTy2, CCLocInfo &pLoc) const override;

    void Init() override
    {
        nextGeneralRegNO = 0;
        nextFloatRegNOF32 = 0;
        nextFloatRegNOF64 = 0;
        nextFloatRegNOF128 = 0;
        nextStackArgAdress = 0;
    }

private:
    BECommon &beCommon;
    int32 nextGeneralRegNO = 0; /* number of integer parameters processed so far */
    uint32 nextFloatRegNOF32 = 0;
    uint32 nextFloatRegNOF64 = 0;
    uint32 nextFloatRegNOF128 = 0;
    static constexpr int32 kNumIntParmRegs = 8;
    static constexpr int32 kNumFloatParmRegsF32 = 4;
    static constexpr int32 kNumFloatParmRegsF64 = 4;
    static constexpr int32 kNumFloatParmRegsF128 = 2;
    static constexpr AArch64reg intParmRegs[kNumIntParmRegs] = {R19, R20, R21, R22, R23, R24, R25, R26};
    static constexpr AArch64reg floatParmRegsF32[kNumFloatParmRegsF32] = {V8, V9, V10, V11};
    static constexpr AArch64reg floatParmRegsF64[kNumFloatParmRegsF64] = {V12, V13, V14, V15};
    static constexpr AArch64reg floatParmRegsF128[kNumFloatParmRegsF128] = {V4, V5};

    int32 ClassificationArg(const BECommon &be, MIRType &mirType, std::vector<ArgumentClass> &classes) const;

    AArch64reg AllocateGPParmRegister()
    {
        DEBUG_ASSERT(nextGeneralRegNO >= 0, "nextGeneralRegNO can not be neg");
        return (nextGeneralRegNO < GHCCC::kNumIntParmRegs) ? GHCCC::intParmRegs[nextGeneralRegNO++] : kRinvalid;
    }

    AArch64reg AllocateSIMDFPParmRegisterF32()
    {
        return (nextFloatRegNOF32 < GHCCC::kNumFloatParmRegsF32) ? GHCCC::floatParmRegsF32[nextFloatRegNOF32++]
                                                                 : kRinvalid;
    }

    AArch64reg AllocateSIMDFPParmRegisterF64()
    {
        return (nextFloatRegNOF64 < GHCCC::kNumFloatParmRegsF64) ? GHCCC::floatParmRegsF64[nextFloatRegNOF64++]
                                                                 : kRinvalid;
    }

    AArch64reg AllocateSIMDFPParmRegisterF128()
    {
        return (nextFloatRegNOF128 < GHCCC::kNumFloatParmRegsF128) ? GHCCC::floatParmRegsF128[nextFloatRegNOF128++]
                                                                   : kRinvalid;
    }
};

} /* namespace maplebe */

#endif /* MAPLEBE_INCLUDE_CG_AARCH64_AARCH64_CALL_CONV_H */
