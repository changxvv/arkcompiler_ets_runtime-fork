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

#ifndef MAPLEBE_INCLUDE_CG_LABEL_CREATION_H
#define MAPLEBE_INCLUDE_CG_LABEL_CREATION_H

#include "cgfunc.h"
#include "cg_phase.h"
#include "mir_builder.h"

namespace maplebe {
class LabelCreation {
public:
    explicit LabelCreation(CGFunc &func) : cgFunc(&func) {}

    ~LabelCreation() = default;

    void Run() const;

    std::string PhaseName() const
    {
        return "createlabel";
    }

private:
    CGFunc *cgFunc;
    void CreateStartEndLabel() const;
};

MAPLE_FUNC_PHASE_DECLARE_BEGIN(CgCreateLabel, maplebe::CGFunc)
MAPLE_FUNC_PHASE_DECLARE_END
} /* namespace maplebe */

#endif /* MAPLEBE_INCLUDE_CG_LABEL_CREATION_H */