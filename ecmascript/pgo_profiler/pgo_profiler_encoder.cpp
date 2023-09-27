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

#include <cerrno>
#include <cstdio>
#include <fstream>
#include <memory>
#include <string>

#include "ecmascript/log_wrapper.h"
#include "ecmascript/pgo_profiler/ap_file/pgo_file_info.h"
#include "ecmascript/pgo_profiler/pgo_profiler_decoder.h"
#include "ecmascript/pgo_profiler/pgo_profiler_encoder.h"

#include "ecmascript/pgo_profiler/pgo_utils.h"
#include "ecmascript/platform/file.h"
#include "os/mutex.h"

namespace panda::ecmascript::pgo {
static const std::string AP_SUFFIX = ".ap";
static const std::string PROFILE_FILE_NAME = "/modules" + AP_SUFFIX;
static const std::string RUNTIME_AP_PREFIX = "/rt_";
void PGOProfilerEncoder::Destroy()
{
    if (!isInitialized_) {
        return;
    }
    PGOProfilerHeader::Destroy(&header_);
    pandaFileInfos_.reset();
    globalRecordInfos_->Clear();
    globalRecordInfos_.reset();
    isInitialized_ = false;
}

bool PGOProfilerEncoder::ResetOutPathByModuleName(const std::string &moduleName)
{
    os::memory::LockHolder lock(mutex_);
    // only first assign takes effect
    if (!moduleName_.empty() || moduleName.empty()) {
        return false;
    }
    moduleName_ = moduleName;
    return ResetOutPath(RUNTIME_AP_PREFIX + moduleName_ + AP_SUFFIX);
}

bool PGOProfilerEncoder::ResetOutPath(const std::string &profileFileName)
{
    if (!RealPath(outDir_, realOutPath_, false)) {
        return false;
    }

    if (realOutPath_.compare(realOutPath_.length() - AP_SUFFIX.length(), AP_SUFFIX.length(), AP_SUFFIX)) {
        realOutPath_ += profileFileName;
    }
    LOG_ECMA(INFO) << "Save profiler to file:" << realOutPath_;
    return true;
}

bool PGOProfilerEncoder::InitializeData()
{
    if (!isInitialized_) {
        if (!ResetOutPath(PROFILE_FILE_NAME)) {
            return false;
        }
        PGOProfilerHeader::Build(&header_, PGOProfilerHeader::LastSize());
        pandaFileInfos_ = std::make_unique<PGOPandaFileInfos>();
        abcFilePool_ = std::make_unique<PGOAbcFilePool>();
        globalRecordInfos_ = std::make_shared<PGORecordDetailInfos>(hotnessThreshold_);
        isInitialized_ = true;
    }
    return true;
}

void PGOProfilerEncoder::SamplePandaFileInfo(uint32_t checksum, const CString &abcName)
{
    if (!isInitialized_) {
        return;
    }
    os::memory::WriteLockHolder lock(rwLock_);
    pandaFileInfos_->Sample(checksum);
    ApEntityId entryId(0);
    abcFilePool_->TryAdd(abcName, entryId);
}

bool PGOProfilerEncoder::GetPandaFileId(const CString &abcName, ApEntityId &entryId)
{
    if (!isInitialized_) {
        return false;
    }
    os::memory::ReadLockHolder lock(rwLock_);
    return abcFilePool_->GetEntryId(abcName, entryId);
}

void PGOProfilerEncoder::Merge(const PGORecordDetailInfos &recordInfos)
{
    if (!isInitialized_) {
        return;
    }
    os::memory::LockHolder lock(mutex_);
    globalRecordInfos_->Merge(recordInfos);
}

void PGOProfilerEncoder::Merge(const PGOPandaFileInfos &pandaFileInfos)
{
    return pandaFileInfos_->Merge(pandaFileInfos);
}

void PGOProfilerEncoder::Merge(const PGOProfilerEncoder &encoder)
{
    Merge(*encoder.pandaFileInfos_);
    Merge(*encoder.globalRecordInfos_);
}

bool PGOProfilerEncoder::VerifyPandaFileMatched(const PGOPandaFileInfos &pandaFileInfos, const std::string &base,
                                                const std::string &incoming) const
{
    return pandaFileInfos_->VerifyChecksum(pandaFileInfos, base, incoming);
}

bool PGOProfilerEncoder::Save()
{
    if (!isInitialized_) {
        return false;
    }
    os::memory::LockHolder lock(mutex_);
    return InternalSave();
}

bool PGOProfilerEncoder::SaveAndRename(const SaveTask *task)
{
    static const char *tempSuffix = ".tmp";
    auto tmpOutPath = realOutPath_ + "." + std::to_string(getpid()) + tempSuffix;
    std::fstream fileStream(tmpOutPath.c_str(),
                            std::fstream::binary | std::fstream::out | std::fstream::in | std::fstream::trunc);
    if (!fileStream.is_open()) {
        LOG_ECMA(ERROR) << "The file path(" << tmpOutPath << ") open failure! errno: " << errno;
        return false;
    }
    pandaFileInfos_->ProcessToBinary(fileStream, header_->GetPandaInfoSection());
    globalRecordInfos_->ProcessToBinary(task, fileStream, header_);
    {
        os::memory::ReadLockHolder lock(rwLock_);
        PGOFileSectionInterface::ProcessSectionToBinary(fileStream, header_, *abcFilePool_->GetPool());
    }
    header_->SetFileSize(static_cast<uint32_t>(fileStream.tellp()));
    header_->ProcessToBinary(fileStream);
    if (header_->SupportFileConsistency()) {
        AddChecksum(fileStream);
    }
    fileStream.close();
    if (task && task->IsTerminate()) {
        LOG_ECMA(DEBUG) << "ProcessProfile: task is already terminate";
        return false;
    }
    if (FileExist(realOutPath_.c_str()) && remove(realOutPath_.c_str())) {
        LOG_ECMA(ERROR) << "Remove " << realOutPath_ << " failure!, errno: " << errno;
        return false;
    }
    if (rename(tmpOutPath.c_str(), realOutPath_.c_str())) {
        LOG_ECMA(ERROR) << "Rename " << tmpOutPath << " --> " << realOutPath_ << " failure!, errno: " << errno;
        return false;
    }
    return true;
}

bool PGOProfilerEncoder::InternalSave(const SaveTask *task)
{
    ECMA_BYTRACE_NAME(HITRACE_TAG_ARK, "PGOProfilerEncoder::InternalSave");
    if (!isInitialized_) {
        return false;
    }
    return SaveAndRename(task);
}

void PGOProfilerEncoder::AddChecksum(std::fstream &fileStream)
{
    static constexpr uint32_t KILO_BYTES = 1024;
    static constexpr uint32_t STEP_IN_KB = 256;
    static constexpr uint32_t STEP_SIZE = STEP_IN_KB * KILO_BYTES;
    uint32_t size = static_cast<uint32_t>(fileStream.seekp(0, std::fstream::end).tellp());
    std::unique_ptr<std::vector<uint8_t>> buffer = std::make_unique<std::vector<uint8_t>>(STEP_SIZE);
    // first, calculate the version field's checksum.
    fileStream.seekg(PGOProfilerHeader::MAGIC_SIZE, std::fstream::beg)
        .read(reinterpret_cast<char *>(buffer->data()), PGOProfilerHeader::VERSION_SIZE);
    uint32_t checksum = adler32(0, reinterpret_cast<const Bytef *>(buffer->data()), PGOProfilerHeader::VERSION_SIZE);
    // second, calculate the checksum for remaining content(exclude checksum field).
    uint32_t remainingSize = size - PGOProfilerHeader::CHECKSUM_END_OFFSET;
    fileStream.seekg(PGOProfilerHeader::CHECKSUM_END_OFFSET);
    while (remainingSize > 0) {
        uint32_t readSize = std::min(STEP_SIZE, remainingSize);
        remainingSize = remainingSize - readSize;
        fileStream.read(reinterpret_cast<char *>(buffer->data()), readSize);
        checksum = adler32(checksum, reinterpret_cast<const Bytef *>(buffer->data()), readSize);
    }
    // third, write the checksum back to the checksum field in the output stream.
    fileStream.seekp(PGOProfilerHeader::MAGIC_SIZE + PGOProfilerHeader::VERSION_SIZE, std::fstream::beg);
    fileStream.write(reinterpret_cast<char *>(&checksum), sizeof(checksum));
}

void PGOProfilerEncoder::TerminateSaveTask()
{
    if (!isInitialized_) {
        return;
    }
    Taskpool::GetCurrentTaskpool()->TerminateTask(GLOBAL_TASK_ID, TaskType::PGO_SAVE_TASK);
}

void PGOProfilerEncoder::PostSaveTask()
{
    if (!isInitialized_) {
        return;
    }
    Taskpool::GetCurrentTaskpool()->PostTask(std::make_unique<SaveTask>(this, GLOBAL_TASK_ID));
}

void PGOProfilerEncoder::StartSaveTask(const SaveTask *task)
{
    if (task == nullptr) {
        return;
    }
    if (task->IsTerminate()) {
        LOG_ECMA(ERROR) << "StartSaveTask: task is already terminate";
        return;
    }
    os::memory::LockHolder lock(mutex_);
    InternalSave(task);
}

bool PGOProfilerEncoder::LoadAPTextFile(const std::string &inPath)
{
    if (!isInitialized_) {
        return false;
    }
    std::string realPath;
    if (!RealPath(inPath, realPath)) {
        return false;
    }

    std::ifstream fileStream(realPath.c_str());
    if (!fileStream.is_open()) {
        LOG_ECMA(ERROR) << "The file path(" << realOutPath_ << ") open failure!";
        return false;
    }

    if (!header_->ParseFromText(fileStream)) {
        LOG_ECMA(ERROR) << "header format error";
        return false;
    }
    if (!pandaFileInfos_->ParseFromText(fileStream)) {
        LOG_ECMA(ERROR) << "panda file info format error";
        return false;
    }
    if (!globalRecordInfos_->ParseFromText(fileStream)) {
        LOG_ECMA(ERROR) << "record info format error";
        return false;
    }

    return true;
}
} // namespace panda::ecmascript::pgo
