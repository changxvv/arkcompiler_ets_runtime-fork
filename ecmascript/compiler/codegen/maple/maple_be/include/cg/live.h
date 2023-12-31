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

#ifndef MAPLEBE_INCLUDE_CG_LIVE_H
#define MAPLEBE_INCLUDE_CG_LIVE_H

#include "cg_phase.h"
#include "insn.h"
#include "cgbb.h"
#include "sparse_datainfo.h"
#include "cgfunc.h"

namespace maplebe {
class LiveAnalysis : public AnalysisResult {
public:
    LiveAnalysis(CGFunc &func, MemPool &memPool)
        : AnalysisResult(&memPool), cgFunc(&func), memPool(&memPool), alloc(&memPool), stackMp(func.GetStackMemPool())
    {
    }
    ~LiveAnalysis() override = default;

    void AnalysisLive();
    void Dump() const;
    void DumpInfo(const SparseDataInfo &info) const;
    void InitBB(BB &bb);
    void InitAndGetDefUse();
    bool GenerateLiveOut(BB &bb);
    bool GenerateLiveIn(BB &bb);
    void BuildInOutforFunc();
    void DealWithInOutOfCleanupBB();
    void InsertInOutOfCleanupBB();
    void ResetLiveSet();
    void ClearInOutDataInfo();
    void EnlargeSpaceForLiveAnalysis(BB &currBB);
    void GetBBDefUse(BB &bb);
    void ProcessAsmListOpnd(BB &bb, Operand &opnd, uint32 idx) const;
    void ProcessListOpnd(BB &bb, Operand &opnd, bool isDef) const;
    void ProcessMemOpnd(BB &bb, Operand &opnd) const;
    void ProcessCondOpnd(BB &bb) const;
    void CollectLiveInfo(BB &bb, const Operand &opnd, bool isDef, bool isUse) const;
    void MarkStackMapInsn(Insn &insn, BB &bb);
    void GenerateStackMapLiveIn();
    SparseDataInfo *GenerateLiveInByDefUse(SparseDataInfo &liveOut, SparseDataInfo &use, SparseDataInfo &def,
                                           const MapleList<BB *> &ehSuccs);

    SparseDataInfo *NewLiveIn(uint32 maxRegCount)
    {
        return memPool->New<SparseDataInfo>(maxRegCount, alloc);
    }

    SparseDataInfo *NewLiveOut(uint32 maxRegCount)
    {
        return memPool->New<SparseDataInfo>(maxRegCount, alloc);
    }

    SparseDataInfo *NewDef(uint32 maxRegCount)
    {
        return memPool->New<SparseDataInfo>(maxRegCount, alloc);
    }

    SparseDataInfo *NewDef(const SparseDataInfo &def)
    {
        return memPool->New<SparseDataInfo>(def, alloc);
    }

    SparseDataInfo *NewUse(uint32 maxRegCount)
    {
        return memPool->New<SparseDataInfo>(maxRegCount, alloc);
    }

    SparseDataInfo *NewUse(const SparseDataInfo &use)
    {
        return memPool->New<SparseDataInfo>(use, alloc);
    }

    virtual void GenerateReturnBBDefUse(BB &bb) const = 0;
    virtual void ProcessCallInsnParam(BB &bb, const Insn &insn) const = 0;
    virtual bool CleanupBBIgnoreReg(regno_t reg) = 0;
    virtual void InitEhDefine(BB &bb) = 0;

protected:
    int iteration = 0;
    CGFunc *cgFunc;
    MemPool *memPool;
    MapleAllocator alloc;
    StackMemPool &stackMp;
};

MAPLE_FUNC_PHASE_DECLARE_BEGIN(CgLiveAnalysis, maplebe::CGFunc)
LiveAnalysis *GetResult()
{
    return live;
}
LiveAnalysis *live = nullptr;
OVERRIDE_DEPENDENCE
MAPLE_FUNC_PHASE_DECLARE_END
} /* namespace maplebe */
#endif /* MAPLEBE_INCLUDE_CG_LIVE_H */
