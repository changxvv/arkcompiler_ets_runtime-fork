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

#ifndef MAPLEBE_INCLUDE_CG_AARCH64_AARCH64_LIVE_H
#define MAPLEBE_INCLUDE_CG_AARCH64_AARCH64_LIVE_H

#include "live.h"

namespace maplebe {
class AArch64LiveAnalysis : public LiveAnalysis {
public:
    AArch64LiveAnalysis(CGFunc &func, MemPool &memPool) : LiveAnalysis(func, memPool) {}
    ~AArch64LiveAnalysis() override = default;
    bool CleanupBBIgnoreReg(regno_t reg) override;
    void GenerateReturnBBDefUse(BB &bb) const override;
    void ProcessCallInsnParam(BB &bb, const Insn &insn) const override;
};
} /* namespace maplebe */

#endif /* MAPLEBE_INCLUDE_CG_AARCH64_AARCH64_LIVE_H */
