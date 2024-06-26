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

#include "x64_optimize_common.h"
#include "x64_cgfunc.h"
#include "cgbb.h"
#include "cg.h"

namespace maplebe {
void X64InsnVisitor::ModifyJumpTarget(Operand &targetOperand, BB &bb)
{
    Insn *jmpInsn = bb.GetLastInsn();
    if (bb.GetKind() == BB::kBBIgoto) {
        CHECK_FATAL(targetOperand.IsLabel(), "NIY");
        CHECK_FATAL(false, "NIY");
    }
    jmpInsn->SetOperand(x64::GetJumpTargetIdx(*jmpInsn), targetOperand);
}

void X64InsnVisitor::ModifyJumpTarget(LabelIdx targetLabel, BB &bb)
{
    std::string lableName = ".L." + std::to_string(GetCGFunc()->GetUniqueID()) + "__" + std::to_string(targetLabel);
    ModifyJumpTarget(GetCGFunc()->GetOpndBuilder()->CreateLabel(lableName.c_str(), targetLabel), bb);
}

void X64InsnVisitor::ModifyJumpTarget(BB &newTarget, BB &bb)
{
    ModifyJumpTarget(newTarget.GetLastInsn()->GetOperand(x64::GetJumpTargetIdx(*newTarget.GetLastInsn())), bb);
}

Insn *X64InsnVisitor::CloneInsn(Insn &originalInsn)
{
    MemPool *memPool = const_cast<MemPool *>(CG::GetCurCGFunc()->GetMemoryPool());
    if (originalInsn.IsTargetInsn()) {
        if (!originalInsn.IsVectorOp()) {
            return memPool->Clone<Insn>(originalInsn);
        } else {
            auto *insn = memPool->Clone<VectorInsn>(*static_cast<VectorInsn *>(&originalInsn));
            insn->SetRegSpecList(static_cast<VectorInsn &>(originalInsn).GetRegSpecList());
            return insn;
        }
    } else if (originalInsn.IsCfiInsn()) {
        return memPool->Clone<cfi::CfiInsn>(*static_cast<cfi::CfiInsn *>(&originalInsn));
    } else if (originalInsn.IsDbgInsn()) {
        return memPool->Clone<mpldbg::DbgInsn>(*static_cast<mpldbg::DbgInsn *>(&originalInsn));
    }
    if (originalInsn.IsComment()) {
        return memPool->Clone<Insn>(originalInsn);
    }
    CHECK_FATAL(false, "Cannot clone");
    return nullptr;
}

/*
 * Precondition: The given insn is a jump instruction.
 * Get the jump target label operand index from the given instruction.
 * Note: MOP_jmp_m, MOP_jmp_r is a jump instruction, but the target is unknown at compile time.
 */
LabelIdx X64InsnVisitor::GetJumpLabel(const Insn &insn) const
{
    uint32 operandIdx = x64::GetJumpTargetIdx(insn);
    if (insn.GetOperand(operandIdx).IsLabelOpnd()) {
        return static_cast<LabelOperand &>(insn.GetOperand(operandIdx)).GetLabelIndex();
    }
    DEBUG_ASSERT(false, "Operand is not label");
    return 0;
}

bool X64InsnVisitor::IsCompareInsn(const Insn &insn) const
{
    switch (insn.GetMachineOpcode()) {
        case x64::MOP_cmpb_r_r:
        case x64::MOP_cmpb_m_r:
        case x64::MOP_cmpb_i_r:
        case x64::MOP_cmpb_r_m:
        case x64::MOP_cmpb_i_m:
        case x64::MOP_cmpw_r_r:
        case x64::MOP_cmpw_m_r:
        case x64::MOP_cmpw_i_r:
        case x64::MOP_cmpw_r_m:
        case x64::MOP_cmpw_i_m:
        case x64::MOP_cmpl_r_r:
        case x64::MOP_cmpl_m_r:
        case x64::MOP_cmpl_i_r:
        case x64::MOP_cmpl_r_m:
        case x64::MOP_cmpl_i_m:
        case x64::MOP_cmpq_r_r:
        case x64::MOP_cmpq_m_r:
        case x64::MOP_cmpq_i_r:
        case x64::MOP_cmpq_r_m:
        case x64::MOP_cmpq_i_m:
        case x64::MOP_testq_r_r:
            return true;
        default:
            return false;
    }
}

bool X64InsnVisitor::IsSimpleJumpInsn(const Insn &insn) const
{
    return (insn.GetMachineOpcode() == x64::MOP_jmpq_l);
}

bool X64InsnVisitor::IsCompareAndBranchInsn(const Insn &insn) const
{
    return false;
}

bool X64InsnVisitor::IsTestAndSetCCInsn(const Insn &insn) const
{
    return false;
}

bool X64InsnVisitor::IsTestAndBranchInsn(const Insn &insn) const
{
    return false;
}

bool X64InsnVisitor::IsAddOrSubInsn(const Insn &insn) const
{
    switch (insn.GetMachineOpcode()) {
        case x64::MOP_addb_r_r:
        case x64::MOP_addw_r_r:
        case x64::MOP_addl_r_r:
        case x64::MOP_addq_r_r:
        case x64::MOP_addb_m_r:
        case x64::MOP_addw_m_r:
        case x64::MOP_addl_m_r:
        case x64::MOP_addq_m_r:
        case x64::MOP_addb_i_r:
        case x64::MOP_addw_i_r:
        case x64::MOP_addl_i_r:
        case x64::MOP_addq_i_r:
        case x64::MOP_addb_r_m:
        case x64::MOP_addw_r_m:
        case x64::MOP_addl_r_m:
        case x64::MOP_addq_r_m:
        case x64::MOP_addb_i_m:
        case x64::MOP_addw_i_m:
        case x64::MOP_addl_i_m:
        case x64::MOP_addq_i_m:
        case x64::MOP_subb_r_r:
        case x64::MOP_subw_r_r:
        case x64::MOP_subl_r_r:
        case x64::MOP_subq_r_r:
        case x64::MOP_subb_m_r:
        case x64::MOP_subw_m_r:
        case x64::MOP_subl_m_r:
        case x64::MOP_subq_m_r:
        case x64::MOP_subb_i_r:
        case x64::MOP_subw_i_r:
        case x64::MOP_subl_i_r:
        case x64::MOP_subq_i_r:
        case x64::MOP_subb_r_m:
        case x64::MOP_subw_r_m:
        case x64::MOP_subl_r_m:
        case x64::MOP_subq_r_m:
        case x64::MOP_subb_i_m:
        case x64::MOP_subw_i_m:
        case x64::MOP_subl_i_m:
            return true;
        default:
            return false;
    }
}

RegOperand *X64InsnVisitor::CreateVregFromReg(const RegOperand &pReg)
{
    return &GetCGFunc()->GetOpndBuilder()->CreateVReg(pReg.GetRegisterNumber(), pReg.GetSize(), pReg.GetRegisterType());
}

void X64InsnVisitor::ReTargetSuccBB(BB &bb, LabelIdx newTarget) const
{
    DEBUG_ASSERT(false, "not implement in X86_64");
    (void)bb;
    (void)newTarget;
    return;
}

void X64InsnVisitor::FlipIfBB(BB &bb, LabelIdx ftLabel) const
{
    DEBUG_ASSERT(false, "not implement in X86_64");
    (void)bb;
    (void)ftLabel;
    return;
}

BB *X64InsnVisitor::CreateGotoBBAfterCondBB(BB &bb, BB &fallthru, bool isTargetFallthru) const
{
    DEBUG_ASSERT(false, "not implement in X86_64");
    (void)bb;
    (void)fallthru;
    (void)isTargetFallthru;
    return nullptr;
}

void X64InsnVisitor::ModifyFathruBBToGotoBB(BB &bb, LabelIdx labelIdx) const
{
    DEBUG_ASSERT(false, "not implement in X86_64");
    (void)bb;
    return;
}
} /* namespace maplebe */
