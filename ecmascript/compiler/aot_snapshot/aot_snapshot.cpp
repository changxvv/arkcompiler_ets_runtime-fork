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

#include "ecmascript/compiler/aot_snapshot/aot_snapshot.h"

#include "ecmascript/common.h"
#include "ecmascript/jspandafile/program_object.h"
#include "ecmascript/log_wrapper.h"
#include "ecmascript/pgo_profiler/pgo_profiler_manager.h"
#include "ecmascript/ts_types/ts_manager.h"
#include "ecmascript/global_env_constants-inl.h"
#include "ecmascript/compiler/aot_snapshot/aot_snapshot_constants.h"

namespace panda::ecmascript::kungfu {
void AOTSnapshot::InitSnapshot(uint32_t compileFilesCount)
{
    JSHandle<TaggedArray> data = factory_->NewTaggedArray(compileFilesCount *
                                                          AOTSnapshotConstants::SNAPSHOT_DATA_ITEM_SIZE);
    snapshotData_.SetData(data.GetTaggedValue());
}

JSHandle<ConstantPool> AOTSnapshot::NewSnapshotConstantPool(uint32_t cacheSize)
{
    JSHandle<ConstantPool> cp = factory_->NewConstantPool(cacheSize);

    ASSERT(!snapshotData_.GetHClassInfo().IsHole());
    cp->SetAotHClassInfo(snapshotData_.GetHClassInfo());
    cp->SetAotArrayInfo(snapshotData_.GetArrayInfo());
    cp->SetConstantIndexInfo(snapshotData_.GetConstantIndexInfo());
    return cp;
}

void AOTSnapshot::GenerateSnapshotConstantPools(const CMap<int32_t, JSTaggedValue> &allConstantPools,
                                                const CString &fileName, uint32_t fileIndex)
{
    JSHandle<TaggedArray> snapshotCpArr = factory_->NewTaggedArray(allConstantPools.size() *
                                                                   AOTSnapshotConstants::SNAPSHOT_CP_ARRAY_ITEM_SIZE);
    snapshotData_.AddSnapshotCpArrayToData(thread_, fileName, fileIndex, snapshotCpArr);

    JSMutableHandle<ConstantPool> cp(thread_, thread_->GlobalConstants()->GetUndefined());
    uint32_t pos = 0;
    for (auto &iter : allConstantPools) {
        int32_t cpId = iter.first;
        cp.Update(iter.second);
        uint32_t cacheSize = cp->GetCacheLength();
        if (vm_->GetJSOptions().IsEnableCompilerLogSnapshot()) {
            LOG_COMPILER(INFO) << "[aot-snapshot] constantPoolID: " << cpId;
            LOG_COMPILER(INFO) << "[aot-snapshot] cacheSize: " << cacheSize;
        }

        JSHandle<ConstantPool> newCp = NewSnapshotConstantPool(cacheSize);
        snapshotCpArr->Set(thread_, pos++, JSTaggedValue(cpId));
        snapshotData_.RecordCpArrIdx(cpId, pos);
        snapshotCpArr->Set(thread_, pos++, newCp.GetTaggedValue());
    }
}

void AOTSnapshot::StoreConstantPoolInfo(BytecodeInfoCollector *bcInfoCollector)
{
    const JSPandaFile *jsPandaFile = bcInfoCollector->GetJSPandaFile();
    const CMap<int32_t, JSTaggedValue> &allConstantPools = vm_->GetJSThread()->
        GetCurrentEcmaContext()->FindConstpools(jsPandaFile).value();
    pgo::ApEntityId fileId = INVALID_INDEX;
    if (!pgo::PGOProfilerManager::GetInstance()->GetPandaFileId(jsPandaFile->GetJSPandaFileDesc(), fileId)) {
        LOG_COMPILER(ERROR) << "StoreConstantPoolInfo failed. no file id found for "
                            << jsPandaFile->GetJSPandaFileDesc();
        return;
    }
    GenerateSnapshotConstantPools(allConstantPools, jsPandaFile->GetNormalizedFileDesc(), fileId);
    bcInfoCollector->StoreDataToGlobalData(snapshotData_);
}
}  // namespace panda::ecmascript
