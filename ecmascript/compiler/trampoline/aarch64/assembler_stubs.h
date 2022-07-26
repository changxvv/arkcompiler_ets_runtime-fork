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

#ifndef ECMASCRIPT_COMPILER_AARCH64_EXTENDED_ASSEMBLER_H
#define ECMASCRIPT_COMPILER_AARCH64_EXTENDED_ASSEMBLER_H

#include "ecmascript/compiler/assembler/aarch64/assembler_aarch64.h"
#include "ecmascript/compiler/assembler/aarch64/extend_assembler.h"
#include "ecmascript/frames.h"

namespace panda::ecmascript::aarch64 {
using Label = panda::ecmascript::Label;
class AssemblerStubs {
public:
    static constexpr int FRAME_SLOT_SIZE = 8;
    static constexpr int DOUBLE_SLOT_SIZE = 16;
    static constexpr int SHIFT_OF_FRAMESLOT = 3;
    enum BuiltinsLeaveFrameArgId : unsigned {CODE_ADDRESS = 0, ENV, ARGC, ARGV};
    static inline int64_t GetStackArgOffSetToFp(unsigned argId)
    {
        //   +--------------------------+
        //   |       argv0              | calltarget , newtARGET, this, ....
        //   +--------------------------+ ---
        //   |       argc               |   ^
        //   |--------------------------|   arguments
        //   |       env                |   |
        //   |--------------------------|   |
        //   |       codeAddress        |   |
        //   |--------------------------|   |
        //   |       returnAddr         |   |
        //   |--------------------------| Fixed OptimizedBuiltinLeaveFrame
        //   |       callsiteFp         |   |
        //   |--------------------------|   |
        //   |       frameType          |   v
        //   +--------------------------+ ---
        // 16 : 16 means arguments offset to fp
        return 16 + argId * FRAME_SLOT_SIZE;
    }

    static void CallRuntime(ExtendedAssembler *assembler);

    static void JSFunctionEntry(ExtendedAssembler *assembler);

    static void OptimizedCallOptimized(ExtendedAssembler *assembler);

    static void CallBuiltinTrampoline(ExtendedAssembler *assembler);

    static void JSProxyCallInternalWithArgV(ExtendedAssembler *assembler);

    static void JSCall(ExtendedAssembler *assembler);

    static void CallRuntimeWithArgv(ExtendedAssembler *assembler);

    static void AsmInterpreterEntry(ExtendedAssembler *assembler);

    static void JSCallDispatch(ExtendedAssembler *assembler);

    static void GeneratorReEnterAsmInterp(ExtendedAssembler *assembler);

    static void GeneratorReEnterAsmInterpDispatch(ExtendedAssembler *assembler);

    static void PushCallIThisRangeAndDispatch(ExtendedAssembler *assembler);

    static void PushCallIRangeAndDispatch(ExtendedAssembler *assembler);

    static void PushCallArgs3AndDispatch(ExtendedAssembler *assembler);

    static void PushCallArgs2AndDispatch(ExtendedAssembler *assembler);

    static void PushCallArgs1AndDispatch(ExtendedAssembler *assembler);

    static void PushCallArgs0AndDispatch(ExtendedAssembler *assembler);

    static void PushCallIThisRangeAndDispatchNative(ExtendedAssembler *assembler);

    static void PushCallIRangeAndDispatchNative(ExtendedAssembler *assembler);

    static void PushCallNewAndDispatchNative(ExtendedAssembler *assembler);

    static void PushCallNewAndDispatch(ExtendedAssembler *assembler);

    static void PushCallArgsAndDispatchNative(ExtendedAssembler *assembler);

    static void ResumeRspAndDispatch(ExtendedAssembler *assembler);

    static void ResumeRspAndReturn([[maybe_unused]] ExtendedAssembler *assembler);

    static void ResumeCaughtFrameAndDispatch(ExtendedAssembler *assembler);

    static void ResumeUncaughtFrameAndReturn(ExtendedAssembler *assembler);

    static void CallGetter(ExtendedAssembler *assembler);

    static void CallSetter(ExtendedAssembler *assembler);

    static void JSCallWithArgV(ExtendedAssembler *assembler);

private:
    static void JSCallBody(ExtendedAssembler *assembler, Register jsfunc);

    static void PushCallThis(ExtendedAssembler *assembler, JSCallMode mode, Label *stackOverflow);

    static Register GetThisRegsiter(ExtendedAssembler *assembler, JSCallMode mode);
    static Register GetNewTargetRegsiter(ExtendedAssembler *assembler, JSCallMode mode);

    static void PushVregs(ExtendedAssembler *assembler, Label *stackOverflow);

    static void DispatchCall(ExtendedAssembler *assembler, Register pc, Register newSp);

    static void CallNativeInternal(ExtendedAssembler *assembler, Register nativeCode);

    static void PushBuiltinFrame(ExtendedAssembler *assembler, Register glue,
        FrameType type, Register op, Register next);

    static void ThrowStackOverflowExceptionAndReturn(ExtendedAssembler *assembler, Register glue, Register fp,
        Register op);

    static void PushFrameState(ExtendedAssembler *assembler, Register prevSp, Register fp, Register currentSlot,
        Register callTarget, Register method, Register pc, Register op);

    static void JSCallCommonEntry(ExtendedAssembler *assembler, JSCallMode mode);
    static void JSCallCommonFastPath(ExtendedAssembler *assembler, JSCallMode mode, Label *pushCallThis,
        Label *stackOverflow);
    static void JSCallCommonSlowPath(ExtendedAssembler *assembler, JSCallMode mode,
                                     Label *fastPathEntry, Label *pushCallThis, Label *stackOverflow);

    static void GetNumVregsFromCallField(ExtendedAssembler *assembler, Register callField, Register numVregs);

    static void GetDeclaredNumArgsFromCallField(ExtendedAssembler *assembler, Register callField,
        Register declaredNumArgs);
    static void PushArgsWithArgv(ExtendedAssembler *assembler, Register glue, Register argc, Register argv,
        Register op, Register fp, Label *next, Label *stackOverflow);
    static void PushUndefinedWithArgc(ExtendedAssembler *assembler, Register glue, Register argc, Register temp,
        Register fp, Label *next, Label *stackOverflow);
    static void StackOverflowCheck(ExtendedAssembler *assembler, Register glue, Register currentSlot, Register numArgs,
        Register op, Label *stackOverflow);
    static void SaveFpAndJumpSize(ExtendedAssembler *assembler, Immediate jumpSize);

    static void PushAsmInterpEntryFrame(ExtendedAssembler *assembler);

    static void PopAsmInterpEntryFrame(ExtendedAssembler *assembler);

    static void PushAsmInterpBridgeFrame(ExtendedAssembler *assembler);

    static void PopAsmInterpBridgeFrame(ExtendedAssembler *assembler);

    static void PushGeneratorFrameState(ExtendedAssembler *assembler, Register &prevSpRegister, Register &fpRegister,
        Register &currentSlotRegister, Register &callTargetRegister, Register &methodRegister,
        Register &contextRegister, Register &pcRegister, Register &operatorRegister);

    static void CallBCStub(ExtendedAssembler *assembler, Register &newSp, Register &glue,
        Register &callTarget, Register &method, Register &pc, Register &temp);

    static void CallNativeEntry(ExtendedAssembler *assembler);

    static void CallNativeWithArgv(ExtendedAssembler *assembler, bool callNew);
    static void OptimizedCallAsmInterpreter(ExtendedAssembler *assembler);
    static void PushArgsWithArgV(ExtendedAssembler *assembler, Register glue, Register jsfunc,
                                 Register actualNumArgs, Register argV, Label *pushCallThis);
    static void CopyArgumentWithArgV(ExtendedAssembler *assembler, Register argc, Register argV);
    static void PushMandatoryJSArgs(ExtendedAssembler *assembler, Register jsfunc,
                                    Register thisObj, Register newTarget, Register currentSp);
    static void PopJSFunctionArgs(ExtendedAssembler *assembler, Register expectedNumArgs, Register actualNumArgs);
    static void PushJSFunctionEntryFrame (ExtendedAssembler *assembler, Register prevFp);
    static void PopJSFunctionEntryFrame(ExtendedAssembler *assembler, Register glue);
    static void PushOptimizedUnfoldArgVFrame(ExtendedAssembler *assembler, Register callSiteSp);
    static void PopOptimizedUnfoldArgVFrame(ExtendedAssembler *assembler);
    static void IncreaseStackForArguments(ExtendedAssembler *assembler, Register argC, Register fp);
    static void PushOptimizedArgsConfigFrame(ExtendedAssembler *assembler);
    static void PopOptimizedArgsConfigFrame(ExtendedAssembler *assembler);
    static void PushLeaveFrame(ExtendedAssembler *assembler, Register glue, bool isBuiltin);
    static void PopLeaveFrame(ExtendedAssembler *assembler, bool isBuiltin);
};
}  // namespace panda::ecmascript::x64
#endif  // ECMASCRIPT_COMPILER_ASSEMBLER_MODULE_X64_H
