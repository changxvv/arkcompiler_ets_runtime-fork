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

#include "litecg.h"
#include "mir_builder.h"
#include "cg_option.h"
#include "mad.h"
#include "cg.h"
#include "maple_phase_support.h"
#include "maple_phase.h"
#include "cg_phasemanager.h"
#include "triple.h"
#include <string>

namespace maple {

namespace litecg {

using namespace maplebe;

LiteCG::LiteCG(Module &mirModule) : module(mirModule)
{
    // Create CGOption: set up default options
    // TODO: should we make CGOptions local?
    cgOptions = &CGOptions::GetInstance();
    cgOptions->EnableLiteCG();
    cgOptions->SetEmitFileType("obj");  // TODO: to kElf
    // cgOptions->SetTarget(X86_64);
    // cgOptions->SetDebug();
    cgOptions->SetQuiet(true);
#if TARGAARCH64
    Triple::GetTriple().Init();
#endif
    // cgOptions->GetDumpPhases().insert("*");
    // cgOptions->FuncFilter("*");
    // cgOptions->SetDefaultOptions(module);

    // module information prepare
    std::string moduleName = module.GetFileName();
    GStrIdx fileNameStrIdx = GlobalTables::GetStrTable().GetOrCreateStrIdxFromName(moduleName);

    // TODO: is this strictly required?
    GStrIdx nameStrIdx = GlobalTables::GetStrTable().GetOrCreateStrIdxFromName("INFO_filename");
    module.PushFileInfoPair(MIRInfoPair(nameStrIdx, fileNameStrIdx.GetIdx()));
    module.PushFileInfoIsString(true);

    module.SetFlavor(kFlavorUnknown);  // TODO: need a new flavor
    // module.SetSrcLang(kSrcLangC);     // TODO: fix this
    module.GetImportFiles().clear();

    // Setup output file name
    module.SetOutputFileName(moduleName + ".s");
}

LiteCG &LiteCG::SetOutputType(OutputType config)
{
    cgOptions->SetEmitFileType((config == kAsm) ? "asm" : "obj");
    return *this;
}

LiteCG &LiteCG::SetTargetType(TargetType config)
{
    // TODO: update target support
    // cgOptions->SetTarget(X86_64);
    return *this;
}

LiteCG &LiteCG::SetDebugType(DebugType config)
{
    // TODO: fix the exposed debug options
    // cgOptions->SetDebug(?);
    return *this;
}

LiteCG &LiteCG::SetVerbose(InfoType config)
{
    cgOptions->SetQuiet((config == kQuiet) ? true : false);
    return *this;
}

void LiteCG::DumpIRToFile(const std::string &fileName)
{
    module.DumpToFile(fileName);
}

LiteCG &LiteCG::SetupLiteCGEmitMemoryManager(
    void *codeSpace, MemoryManagerAllocateDataSectionCallback dataSectionAllocator,
    MemoryManagerSaveFunc2AddressInfoCallback funcAddressSaver,
    maplebe::MemoryManagerSaveFunc2FPtoPrevSPDeltaCallback funcFpSPDeltaSaver,
    maplebe::MemoryManagerSaveFunc2CalleeOffsetInfoCallback funcCallOffsetSaver,
    maplebe::MemoryManagerSavePC2DeoptInfoCallback pc2DeoptInfoSaver,
    maplebe::MemoryManagerSavePC2CallSiteInfoCallback pc2CallSiteInfoSaver)
{
    cgOptions->SetupEmitMemoryManager(codeSpace, dataSectionAllocator, funcAddressSaver, funcFpSPDeltaSaver,
                                      funcCallOffsetSaver, pc2DeoptInfoSaver, pc2CallSiteInfoSaver);
    return *this;
}

void LiteCG::DoCG()
{
    bool timePhases = false;
    // MPLTimer timer;
    // timer.Start();

    Globals::GetInstance()->SetOptimLevel(cgOptions->GetOptimizeLevel());

    // TODO: not sure how to do this.
    auto cgPhaseManager = std::make_unique<ThreadLocalMemPool>(memPoolCtrler, "cg function phasemanager");
    const MaplePhaseInfo *cgPMInfo = MaplePhaseRegister::GetMaplePhaseRegister()->GetPhaseByID(&CgFuncPM::id);
    auto *cgfuncPhaseManager = static_cast<CgFuncPM *>(cgPMInfo->GetConstructor()(cgPhaseManager.get()));
    cgfuncPhaseManager->SetQuiet(CGOptions::IsQuiet());

    if (timePhases) {
        cgfuncPhaseManager->InitTimeHandler();
    }

    /* It is a specifc work around  (need refactor) */
    cgfuncPhaseManager->SetCGOptions(cgOptions);
    (void)cgfuncPhaseManager->PhaseRun(module);

    if (timePhases) {
        cgfuncPhaseManager->DumpPhaseTime();
    }
    // timer.Stop();
    // LogInfo::MapleLogger() << "Mplcg consumed " << timer.ElapsedMilliseconds() << "ms" << '\n';
}

}  // namespace litecg

}  // namespace maple
