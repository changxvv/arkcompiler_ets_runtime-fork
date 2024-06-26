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

#ifndef MAPLEBE_INCLUDE_CG_RAOPT_H
#define MAPLEBE_INCLUDE_CG_RAOPT_H

#include "cgfunc.h"
#include "cg_phase.h"
#include "loop.h"

namespace maplebe {
class RaOpt {
public:
    RaOpt(CGFunc &func, MemPool &pool, DomAnalysis &dom, LoopAnalysis &loop)
        : cgFunc(&func), memPool(&pool), domInfo(dom), loopInfo(loop)
    {
    }

    virtual ~RaOpt() = default;

    virtual void Run() {}

    std::string PhaseName() const
    {
        return "raopt";
    }

    const CGFunc *GetCGFunc() const
    {
        return cgFunc;
    }
    const MemPool *GetMemPool() const
    {
        return memPool;
    }

protected:
    CGFunc *cgFunc;
    MemPool *memPool;
    DomAnalysis &domInfo;
    LoopAnalysis &loopInfo;
};

MAPLE_FUNC_PHASE_DECLARE_BEGIN(CgRaOpt, maplebe::CGFunc)
OVERRIDE_DEPENDENCE
MAPLE_FUNC_PHASE_DECLARE_END
} /* namespace maplebe */

#endif /* MAPLEBE_INCLUDE_CG_RAOPT_H */
