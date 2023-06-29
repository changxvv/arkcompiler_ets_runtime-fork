/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ECMASCRIPT_JSPANDAFILE_DEBUG_INFO_EXTRACTOR_H
#define ECMASCRIPT_JSPANDAFILE_DEBUG_INFO_EXTRACTOR_H

#include <mutex>

#include "ecmascript/common.h"
#include "ecmascript/debugger/js_pt_location.h"
#include "ecmascript/jspandafile/js_pandafile.h"
#include "ecmascript/mem/c_containers.h"
#include "ecmascript/mem/c_string.h"

#include "libpandafile/class_data_accessor-inl.h"
#include "libpandafile/file.h"
#include "libpandabase/utils/utf.h"

namespace panda::ecmascript {
class JSPandaFile;

struct LineTableEntry {
    uint32_t offset;
    int32_t line;

    bool operator<(const LineTableEntry &other) const
    {
        return offset < other.offset;
    }
};

struct ColumnTableEntry {
    uint32_t offset;
    int32_t column;

    bool operator<(const ColumnTableEntry &other) const
    {
        return offset < other.offset;
    }
};

using LineNumberTable = CVector<LineTableEntry>;
using ColumnNumberTable = CVector<ColumnTableEntry>;
using JSPtLocation = tooling::JSPtLocation;

/*
 * Full version of LocalVariableInfo is defined in frontend,
 * here only using name, reg_number, start_offset, and end_offset:
 *   std::string name
 *   std::string type
 *   std::string typeSignature
 *   int32_t regNumber
 *   uint32_t startOffset
 *   uint32_t endOffset
 */
struct LocalVariableInfo {
    std::string name;
    int32_t reg_number;
    uint32_t start_offset;
    uint32_t end_offset;
};
using LocalVariableTable = CVector<LocalVariableInfo>;

// public for debugger
class PUBLIC_API DebugInfoExtractor {
public:
    explicit DebugInfoExtractor(const JSPandaFile *jsPandaFile) : jsPandaFile_(jsPandaFile)
    {}

    ~DebugInfoExtractor() = default;

    const LineNumberTable &GetLineNumberTable(const panda_file::File::EntityId methodId);

    const ColumnNumberTable &GetColumnNumberTable(const panda_file::File::EntityId methodId);

    const LocalVariableTable &GetLocalVariableTable(const panda_file::File::EntityId methodId);

    const std::string &GetSourceFile(const panda_file::File::EntityId methodId);

    const std::string &GetSourceCode(const panda_file::File::EntityId methodId);

    template<class Callback>
    bool MatchWithLocation(const Callback &cb, int32_t line, int32_t column,
        const std::string &url, const std::string &debugRecordName)
    {
        if (line == SPECIAL_LINE_MARK) {
            return false;
        }
        auto &pandaFile = *jsPandaFile_->GetPandaFile();
        auto classes = jsPandaFile_->GetClasses();
        for (size_t i = 0; i < classes.Size(); i++) {
            panda_file::File::EntityId id(classes[i]);
            if (jsPandaFile_->IsExternal(id)) {
                continue;
            }

            CVector<panda_file::File::EntityId> methodIds;
            panda_file::ClassDataAccessor cda(pandaFile, id);
            CString recordName = JSPandaFile::ParseEntryPoint(utf::Mutf8AsCString(cda.GetDescriptor()));
            // the recordName for testcases is empty
            if (!debugRecordName.empty() && recordName != debugRecordName.c_str()
                && debugRecordName != JSPandaFile::ENTRY_MAIN_FUNCTION) {
                continue;
            }
            cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
                methodIds.push_back(mda.GetMethodId());
            });

            for (auto &methodId : methodIds) {
                const std::string &sourceFile = GetSourceFile(methodId);
                // the url for testcases is empty
                if (!url.empty() && sourceFile != url) {
                    continue;
                }
                const LineNumberTable &lineTable = GetLineNumberTable(methodId);
                const ColumnNumberTable &columnTable = GetColumnNumberTable(methodId);
                for (uint32_t j = 0; j < lineTable.size(); j++) {
                    if (lineTable[j].line != line) {
                        continue;
                    }
                    uint32_t currentOffset = lineTable[j].offset;
                    uint32_t nextOffset = ((j == lineTable.size() - 1) ? UINT32_MAX : lineTable[j + 1].offset);
                    for (const auto &pair : columnTable) {
                        if (pair.column == column && pair.offset >= currentOffset && pair.offset < nextOffset) {
                            return cb(JSPtLocation(jsPandaFile_, methodId, pair.offset, url));
                        }
                    }
                    return cb(JSPtLocation(jsPandaFile_, methodId, currentOffset, url));
                }
            }
        }
        return false;
    }

    template<class Callback>
    bool MatchLineWithOffset(const Callback &cb, panda_file::File::EntityId methodId, uint32_t offset)
    {
        int32_t line = 0;
        const LineNumberTable &lineTable = GetLineNumberTable(methodId);
        auto iter = std::upper_bound(lineTable.begin(), lineTable.end(), LineTableEntry {offset, 0});
        if (iter != lineTable.begin()) {
            line = (iter - 1)->line;
        }
        return cb(line);
    }

    template<class Callback>
    bool MatchColumnWithOffset(const Callback &cb, panda_file::File::EntityId methodId, uint32_t offset)
    {
        int32_t column = 0;
        const ColumnNumberTable &columnTable = GetColumnNumberTable(methodId);
        auto iter = std::upper_bound(columnTable.begin(), columnTable.end(), ColumnTableEntry {offset, 0});
        if (iter != columnTable.begin()) {
            column = (iter - 1)->column;
        }
        return cb(column);
    }

#if defined(ECMASCRIPT_SUPPORT_CPUPROFILER) || defined(ECMASCRIPT_SUPPORT_HEAPSAMPLING)
    int32_t GetFristLine(panda_file::File::EntityId methodId)
    {
        const LineNumberTable &lineTable = GetLineNumberTable(methodId);
        if (lineTable.size() <= 1) {
            return 0;
        }
        return lineTable[1].line + 1;
    }

    int32_t GetFristColumn(panda_file::File::EntityId methodId)
    {
        const ColumnNumberTable &columnTable = GetColumnNumberTable(methodId);
        if (columnTable.size() == 0) {
            return 0;
        }
        return columnTable[0].column + 1;
    }
#endif

    void Extract();

    constexpr static int32_t SPECIAL_LINE_MARK = -1;

private:
    bool ExtractorMethodDebugInfo(const panda_file::File::EntityId methodId);
    void ExtractorMethodDebugInfo(const panda_file::File &pandaFile,
                                  const std::optional<panda_file::File::EntityId> sourceFileId,
                                  const std::optional<panda_file::File::EntityId> debugInfoId,
                                  uint32_t offset);
    struct MethodDebugInfo {
        std::string sourceFile;
        std::string sourceCode;
        LineNumberTable lineNumberTable;
        ColumnNumberTable columnNumberTable;
        LocalVariableTable localVariableTable;
    };

    std::recursive_mutex mutex_;
    CUnorderedMap<uint32_t, MethodDebugInfo> methods_;
    const JSPandaFile *jsPandaFile_ {nullptr};
};
}  // namespace panda::ecmascript

#endif  // ECMASCRIPT_JSPANDAFILE_DEBUG_INFO_EXTRACTOR_H
