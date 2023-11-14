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

#ifndef MAPLEBE_INCLUDE_CG_CG_CFG_H
#define MAPLEBE_INCLUDE_CG_CG_CFG_H
#include "eh_func.h"
#include "cgbb.h"

namespace maplebe {
class InsnVisitor {
public:
    explicit InsnVisitor(CGFunc &func) : cgFunc(&func) {}

    virtual ~InsnVisitor() = default;
    CGFunc *GetCGFunc() const
    {
        return cgFunc;
    }

    /*
     * Precondition:
     * The last instruction in bb is either conditional or unconditional jump.
     *
     * The jump target of bb is modified to the location specified by targetLabel.
     */
    virtual void ModifyJumpTarget(LabelIdx targetLabel, BB &bb) = 0;

    /*
     * Precondition:
     * The last instruction in bb is either conditional or unconditional jump.
     *
     * The jump target of bb is modified to the location specified by targetOperand.
     */
    virtual void ModifyJumpTarget(Operand &targetOperand, BB &bb) = 0;

    /*
     * Precondition:
     * The last instruction in bb is either a conditional or an unconditional jump.
     * The last instruction in newTarget is an unconditional jump.
     *
     * The jump target of bb is modified to newTarget's jump target.
     */
    virtual void ModifyJumpTarget(BB &newTarget, BB &bb) = 0;
    /* Check if it requires to add extra gotos when relocate bb */
    virtual Insn *CloneInsn(Insn &originalInsn) = 0;
    /* Create a new virtual register operand which has the same type and size as the given one. */
    virtual RegOperand *CreateVregFromReg(const RegOperand &reg) = 0;
    virtual LabelIdx GetJumpLabel(const Insn &insn) const = 0;
    virtual bool IsCompareInsn(const Insn &insn) const = 0;
    virtual bool IsCompareAndBranchInsn(const Insn &insn) const = 0;
    virtual bool IsAddOrSubInsn(const Insn &insn) const = 0;

private:
    CGFunc *cgFunc;
}; /* class InsnVisitor; */

class CGCFG {
public:
    explicit CGCFG(CGFunc &cgFunc) : cgFunc(&cgFunc) {}

    ~CGCFG() = default;

    void BuildCFG();
    void CheckCFG();
    void CheckCFGFreq();

    void InitInsnVisitor(CGFunc &func);
    InsnVisitor *GetInsnModifier() const
    {
        return insnVisitor;
    }

    static bool AreCommentAllPreds(const BB &bb);
    bool CanMerge(const BB &merger, const BB &mergee) const;
    bool BBJudge(const BB &first, const BB &second) const;
    /*
     * Merge all instructions in mergee into merger, each BB's successors and
     * predecessors should be modified accordingly.
     */
    static void MergeBB(BB &merger, BB &mergee, CGFunc &func);

    /*
     * Remove a BB from its position in the CFG.
     * Prev, next, preds and sucs are all modified accordingly.
     */
    void RemoveBB(BB &curBB, bool isGotoIf = false);
    /* Skip the successor of bb, directly jump to bb's successor'ssuccessor */
    void RetargetJump(BB &srcBB, BB &targetBB);

    /* Loop up if the given label is in the exception tables in LSDA */
    static bool InLSDA(LabelIdx label, const EHFunc &ehFunc);
    static bool InSwitchTable(LabelIdx label, const CGFunc &func);

    RegOperand *CreateVregFromReg(const RegOperand &pReg);
    Insn *CloneInsn(Insn &originalInsn);
    static BB *GetTargetSuc(BB &curBB, bool branchOnly = false, bool isGotoIf = false);
    bool IsCompareAndBranchInsn(const Insn &insn) const;
    bool IsAddOrSubInsn(const Insn &insn) const;

    Insn *FindLastCondBrInsn(BB &bb) const;
    static void FindAndMarkUnreachable(CGFunc &func);
    void FlushUnReachableStatusAndRemoveRelations(BB &bb, const CGFunc &func) const;
    void MarkLabelTakenBB();
    void UnreachCodeAnalysis();
    void FindWillExitBBs(BB *bb, std::set<BB *, BBIdCmp> *visitedBBs);
    void WontExitAnalysis();
    BB *FindLastRetBB();

    void UpdatePredsSuccsAfterSplit(BB &pred, BB &succ, BB &newBB);
    void BreakCriticalEdge(BB &pred, BB &succ);
    /* cgcfgvisitor */
private:
    CGFunc *cgFunc = nullptr;
    static InsnVisitor *insnVisitor;
    static void MergeBB(BB &merger, BB &mergee);
}; /* class CGCFG */
MAPLE_FUNC_PHASE_DECLARE_BEGIN(CgHandleCFG, maplebe::CGFunc)
MAPLE_FUNC_PHASE_DECLARE_END
} /* namespace maplebe */

#endif /* MAPLEBE_INCLUDE_CG_CG_CFG_H */
