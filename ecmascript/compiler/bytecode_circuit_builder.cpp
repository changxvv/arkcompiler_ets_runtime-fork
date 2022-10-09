/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "ecmascript/compiler/bytecode_circuit_builder.h"

#include "ecmascript/base/number_helper.h"
#include "ecmascript/compiler/gate_accessor.h"
#include "ecmascript/ts_types/ts_manager.h"
#include "libpandafile/bytecode_instruction-inl.h"

namespace panda::ecmascript::kungfu {
void BytecodeCircuitBuilder::BytecodeToCircuit()
{
    std::map<std::pair<uint8_t *, uint8_t *>, std::vector<uint8_t *>> exceptionInfo;

    // collect try catch block info
    CollectTryCatchBlockInfo(exceptionInfo);

    // Complete bytecode block Information
    CompleteBytecodeBlockInfo();

    // Building the basic block diagram of bytecode
    BuildBasicBlocks(exceptionInfo);
}

void BytecodeCircuitBuilder::CollectBytecodeBlockInfo(uint8_t *pc, std::vector<CfgInfo> &bytecodeBlockInfos)
{
    BytecodeInstruction inst(pc);
    auto opcode = inst.GetOpcode();
    auto bytecodeOffset = BytecodeInstruction::Size(opcode);
    switch (static_cast<EcmaOpcode>(opcode)) {
        case EcmaOpcode::JMP_IMM8: {
            int8_t offset = static_cast<int8_t>(READ_INST_8_0());
            std::vector<uint8_t *> temp;
            temp.emplace_back(pc + offset);
            // current basic block end
            bytecodeBlockInfos.emplace_back(pc, SplitKind::END, temp);
            bytecodeBlockInfos.emplace_back(pc + bytecodeOffset, SplitKind::START,
                                            std::vector<uint8_t *>(1, pc + bytecodeOffset));
            // jump basic block start
            bytecodeBlockInfos.emplace_back(pc + offset, SplitKind::START, std::vector<uint8_t *>(1, pc + offset));
        }
            break;
        case EcmaOpcode::JMP_IMM16: {
            int16_t offset = static_cast<int16_t>(READ_INST_16_0());
            std::vector<uint8_t *> temp;
            temp.emplace_back(pc + offset);
            bytecodeBlockInfos.emplace_back(pc, SplitKind::END, temp);
            bytecodeBlockInfos.emplace_back(pc + bytecodeOffset, SplitKind::START,
                                            std::vector<uint8_t *>(1, pc + bytecodeOffset));
            bytecodeBlockInfos.emplace_back(pc + offset, SplitKind::START, std::vector<uint8_t *>(1, pc + offset));
        }
            break;
        case EcmaOpcode::JMP_IMM32: {
            int32_t offset = static_cast<int32_t>(READ_INST_32_0());
            std::vector<uint8_t *> temp;
            temp.emplace_back(pc + offset);
            bytecodeBlockInfos.emplace_back(pc, SplitKind::END, temp);
            bytecodeBlockInfos.emplace_back(pc + bytecodeOffset, SplitKind::START,
                                            std::vector<uint8_t *>(1, pc + bytecodeOffset));
            bytecodeBlockInfos.emplace_back(pc + offset, SplitKind::START, std::vector<uint8_t *>(1, pc + offset));
        }
            break;
        case EcmaOpcode::JEQZ_IMM8: {
            std::vector<uint8_t *> temp;
            temp.emplace_back(pc + bytecodeOffset);   // first successor
            int8_t offset = static_cast<int8_t>(READ_INST_8_0());
            temp.emplace_back(pc + offset);  // second successor
            // condition branch current basic block end
            bytecodeBlockInfos.emplace_back(pc, SplitKind::END, temp);
            // first branch basic block start
            bytecodeBlockInfos.emplace_back(pc + bytecodeOffset, SplitKind::START,
                                            std::vector<uint8_t *>(1, pc + bytecodeOffset));
            // second branch basic block start
            bytecodeBlockInfos.emplace_back(pc + offset, SplitKind::START, std::vector<uint8_t *>(1, pc + offset));
        }
            break;
        case EcmaOpcode::JEQZ_IMM16: {
            std::vector<uint8_t *> temp;
            temp.emplace_back(pc + bytecodeOffset);   // first successor
            int16_t offset = static_cast<int16_t>(READ_INST_16_0());
            temp.emplace_back(pc + offset);  // second successor
            bytecodeBlockInfos.emplace_back(pc, SplitKind::END, temp); // end
            bytecodeBlockInfos.emplace_back(pc + bytecodeOffset, SplitKind::START,
                                            std::vector<uint8_t *>(1, pc + bytecodeOffset));
            bytecodeBlockInfos.emplace_back(pc + offset, SplitKind::START, std::vector<uint8_t *>(1, pc + offset));
        }
            break;
        case EcmaOpcode::JEQZ_IMM32: {
            std::vector<uint8_t *> temp;
            temp.emplace_back(pc + bytecodeOffset);   // first successor
            int16_t offset = static_cast<int16_t>(READ_INST_32_0());
            temp.emplace_back(pc + offset);  // second successor
            bytecodeBlockInfos.emplace_back(pc, SplitKind::END, temp); // end
            bytecodeBlockInfos.emplace_back(pc + bytecodeOffset, SplitKind::START,
                                            std::vector<uint8_t *>(1, pc + bytecodeOffset));
            bytecodeBlockInfos.emplace_back(pc + offset, SplitKind::START, std::vector<uint8_t *>(1, pc + offset));
        }
            break;
        case EcmaOpcode::JNEZ_IMM8: {
            std::vector<uint8_t *> temp;
            temp.emplace_back(pc + bytecodeOffset); // first successor
            int8_t offset = static_cast<int8_t>(READ_INST_8_0());
            temp.emplace_back(pc + offset); // second successor
            bytecodeBlockInfos.emplace_back(pc, SplitKind::END, temp);
            bytecodeBlockInfos.emplace_back(pc + bytecodeOffset, SplitKind::START,
                                            std::vector<uint8_t *>(1, pc + bytecodeOffset));
            bytecodeBlockInfos.emplace_back(pc + offset, SplitKind::START, std::vector<uint8_t *>(1, pc + offset));
        }
            break;
        case EcmaOpcode::JNEZ_IMM16: {
            std::vector<uint8_t *> temp;
            temp.emplace_back(pc + bytecodeOffset); // first successor
            int16_t offset = static_cast<int16_t>(READ_INST_16_0());
            temp.emplace_back(pc + offset); // second successor
            bytecodeBlockInfos.emplace_back(pc, SplitKind::END, temp);
            bytecodeBlockInfos.emplace_back(pc + bytecodeOffset, SplitKind::START,
                                            std::vector<uint8_t *>(1, pc + bytecodeOffset));
            bytecodeBlockInfos.emplace_back(pc + offset, SplitKind::START, std::vector<uint8_t *>(1, pc + offset));
        }
            break;
        case EcmaOpcode::JNEZ_IMM32: {
            std::vector<uint8_t *> temp;
            temp.emplace_back(pc + bytecodeOffset); // first successor
            int16_t offset = static_cast<int16_t>(READ_INST_32_0());
            temp.emplace_back(pc + offset); // second successor
            bytecodeBlockInfos.emplace_back(pc, SplitKind::END, temp);
            bytecodeBlockInfos.emplace_back(pc + bytecodeOffset, SplitKind::START,
                                            std::vector<uint8_t *>(1, pc + bytecodeOffset));
            bytecodeBlockInfos.emplace_back(pc + offset, SplitKind::START, std::vector<uint8_t *>(1, pc + offset));
        }
            break;
        case EcmaOpcode::RETURN:
        case EcmaOpcode::RETURNUNDEFINED:
        case EcmaOpcode::THROW_PREF_NONE:
        case EcmaOpcode::THROW_CONSTASSIGNMENT_PREF_V8:
        case EcmaOpcode::THROW_NOTEXISTS_PREF_NONE:
        case EcmaOpcode::THROW_PATTERNNONCOERCIBLE_PREF_NONE:
        case EcmaOpcode::THROW_DELETESUPERPROPERTY_PREF_NONE: {
            bytecodeBlockInfos.emplace_back(pc, SplitKind::END, std::vector<uint8_t *>(1, pc));
            break;
        }
        default:
            break;
    }
}

void BytecodeCircuitBuilder::CollectTryCatchBlockInfo(std::map<std::pair<uint8_t *, uint8_t *>,
                                                      std::vector<uint8_t *>> &byteCodeException)
{
    // try contains many catch
    panda_file::MethodDataAccessor mda(*pf_, method_->GetMethodId());
    panda_file::CodeDataAccessor cda(*pf_, mda.GetCodeId().value());

    cda.EnumerateTryBlocks([this, &byteCodeException](
            panda_file::CodeDataAccessor::TryBlock &try_block) {
        auto tryStartOffset = try_block.GetStartPc();
        auto tryEndOffset = try_block.GetStartPc() + try_block.GetLength();
        auto tryStartPc = const_cast<uint8_t *>(method_->GetBytecodeArray() + tryStartOffset);
        auto tryEndPc = const_cast<uint8_t *>(method_->GetBytecodeArray() + tryEndOffset);
        // skip try blocks with same pc in start and end label
        if (tryStartPc == tryEndPc) {
            return true;
        }
        byteCodeException[std::make_pair(tryStartPc, tryEndPc)] = {};
        uint32_t pcOffset = panda_file::INVALID_OFFSET;
        try_block.EnumerateCatchBlocks([&](panda_file::CodeDataAccessor::CatchBlock &catch_block) {
            pcOffset = catch_block.GetHandlerPc();
            auto catchBlockPc = const_cast<uint8_t *>(method_->GetBytecodeArray() + pcOffset);
            // try block associate catch block
            byteCodeException[std::make_pair(tryStartPc, tryEndPc)].emplace_back(catchBlockPc);
            return true;
        });
        // Check whether the previous block of the try block exists.
        // If yes, add the current block; otherwise, create a new block.
        bool flag = false;
        for (size_t i = 0; i < bytecodeBlockInfos_.size(); i++) {
            if (bytecodeBlockInfos_[i].splitKind == SplitKind::START) {
                continue;
            }
            if (bytecodeBlockInfos_[i].pc == byteCodeCurPrePc_.at(tryStartPc)) {
                flag = true;
                break;
            }
        }
        if (!flag) {
            // pre block
            if (byteCodeCurPrePc_.at(tryStartPc) != tryStartPc) {
                bytecodeBlockInfos_.emplace_back(byteCodeCurPrePc_.at(tryStartPc), SplitKind::END,
                                             std::vector<uint8_t *>(1, tryStartPc));
            }
        }
        // try block
        bytecodeBlockInfos_.emplace_back(tryStartPc, SplitKind::START, std::vector<uint8_t *>(1, tryStartPc));
        flag = false;
        for (size_t i = 0; i < bytecodeBlockInfos_.size(); i++) {
            if (bytecodeBlockInfos_[i].splitKind == SplitKind::START) {
                continue;
            }
            if (bytecodeBlockInfos_[i].pc == byteCodeCurPrePc_.at(tryEndPc)) {
                auto &succs = bytecodeBlockInfos_[i].succs;
                auto iter = std::find(succs.cbegin(), succs.cend(), bytecodeBlockInfos_[i].pc);
                if (iter == succs.cend()) {
                    auto opcode = PcToOpcode(bytecodeBlockInfos_[i].pc);
                    switch (opcode) {
                        case EcmaOpcode::JMP_IMM8:
                        case EcmaOpcode::JMP_IMM16:
                        case EcmaOpcode::JMP_IMM32:
                        case EcmaOpcode::JEQZ_IMM8:
                        case EcmaOpcode::JEQZ_IMM16:
                        case EcmaOpcode::JEQZ_IMM32:
                        case EcmaOpcode::JNEZ_IMM8:
                        case EcmaOpcode::JNEZ_IMM16:
                        case EcmaOpcode::JNEZ_IMM32:
                        case EcmaOpcode::RETURN:
                        case EcmaOpcode::RETURNUNDEFINED:
                        case EcmaOpcode::THROW_PREF_NONE: {
                            break;
                        }
                        default: {
                            succs.emplace_back(tryEndPc);
                            break;
                        }
                    }
                }
                flag = true;
                break;
            }
        }
        if (!flag) {
            bytecodeBlockInfos_.emplace_back(byteCodeCurPrePc_.at(tryEndPc), SplitKind::END,
                                             std::vector<uint8_t *>(1, tryEndPc));
        }
        bytecodeBlockInfos_.emplace_back(tryEndPc, SplitKind::START, std::vector<uint8_t *>(1, tryEndPc)); // next block
        return true;
    });
}

void BytecodeCircuitBuilder::CompleteBytecodeBlockInfo()
{
    std::sort(bytecodeBlockInfos_.begin(), bytecodeBlockInfos_.end());

    // Deduplicate
    auto deduplicateIndex = std::unique(bytecodeBlockInfos_.begin(), bytecodeBlockInfos_.end());
    bytecodeBlockInfos_.erase(deduplicateIndex, bytecodeBlockInfos_.end());

    // Supplementary block information
    // endBlockPc: Pairs occur, with odd indexes indicating endPc, and even indexes indicating startPc.
    std::vector<uint8_t *> endBlockPc;
    std::vector<uint8_t *> startBlockPc; //
    for (size_t i = 0; i < bytecodeBlockInfos_.size() - 1; i++) {
        if (bytecodeBlockInfos_[i].splitKind == bytecodeBlockInfos_[i + 1].splitKind &&
            bytecodeBlockInfos_[i].splitKind == SplitKind::START) {
            auto prePc = byteCodeCurPrePc_.at(bytecodeBlockInfos_[i + 1].pc);
            endBlockPc.emplace_back(prePc); // Previous instruction of current instruction
            endBlockPc.emplace_back(bytecodeBlockInfos_[i + 1].pc); // current instruction
            continue;
        }
        if (bytecodeBlockInfos_[i].splitKind == bytecodeBlockInfos_[i + 1].splitKind &&
            bytecodeBlockInfos_[i].splitKind == SplitKind::END) {
            auto tempPc = bytecodeBlockInfos_[i].pc;
            auto findItem = std::find_if(byteCodeCurPrePc_.cbegin(), byteCodeCurPrePc_.cend(),
                                         [tempPc](const std::map<uint8_t *, uint8_t *>::value_type item) {
                                             return item.second == tempPc;
                                         });
            if (findItem != byteCodeCurPrePc_.cend()) {
                startBlockPc.emplace_back((*findItem).first);
            }
        }
    }

    // Supplementary end block info
    for (auto iter = endBlockPc.cbegin(); iter != endBlockPc.cend(); iter += 2) { // 2: index
        bytecodeBlockInfos_.emplace_back(*iter, SplitKind::END, std::vector<uint8_t *>(1, *(iter + 1)));
    }
    // Supplementary start block info
    for (auto iter = startBlockPc.cbegin(); iter != startBlockPc.cend(); iter++) {
        bytecodeBlockInfos_.emplace_back(*iter, SplitKind::START, std::vector<uint8_t *>(1, *iter));
    }

    // Deduplicate successor
    for (size_t i = 0; i < bytecodeBlockInfos_.size(); i++) {
        if (bytecodeBlockInfos_[i].splitKind == SplitKind::END) {
            std::set<uint8_t *> tempSet(bytecodeBlockInfos_[i].succs.cbegin(),
                                        bytecodeBlockInfos_[i].succs.cend());
            bytecodeBlockInfos_[i].succs.assign(tempSet.cbegin(), tempSet.cend());
        }
    }

    std::sort(bytecodeBlockInfos_.begin(), bytecodeBlockInfos_.end());

    // handling jumps to an empty block
    auto endPc = bytecodeBlockInfos_[bytecodeBlockInfos_.size() - 1].pc;
    auto iter = --byteCodeCurPrePc_.cend();
    if (endPc == iter->first) {
        bytecodeBlockInfos_.emplace_back(endPc, SplitKind::END, std::vector<uint8_t *>(1, endPc));
    }
    // Deduplicate
    deduplicateIndex = std::unique(bytecodeBlockInfos_.begin(), bytecodeBlockInfos_.end());
    bytecodeBlockInfos_.erase(deduplicateIndex, bytecodeBlockInfos_.end());
}

void BytecodeCircuitBuilder::BuildBasicBlocks(std::map<std::pair<uint8_t *, uint8_t *>,
                                                       std::vector<uint8_t *>> &exception)
{
    std::map<uint8_t *, BytecodeRegion *> startPcToBB; // [start, bb]
    std::map<uint8_t *, BytecodeRegion *> endPcToBB; // [end, bb]
    graph_.resize(bytecodeBlockInfos_.size() / 2); // 2 : half size
    // build basic block
    int blockId = 0;
    int index = 0;
    for (size_t i = 0; i < bytecodeBlockInfos_.size() - 1; i += 2) { // 2:index
        auto startPc = bytecodeBlockInfos_[i].pc;
        auto endPc = bytecodeBlockInfos_[i + 1].pc;
        auto block = &graph_[index++];
        block->id = blockId++;
        block->start = startPc;
        block->end = endPc;
        block->preds = {};
        block->succs = {};
        startPcToBB[startPc] = block;
        endPcToBB[endPc] = block;
    }

    // add block associate
    for (size_t i = 0; i < bytecodeBlockInfos_.size(); i++) {
        if (bytecodeBlockInfos_[i].splitKind == SplitKind::START) {
            continue;
        }
        auto curPc = bytecodeBlockInfos_[i].pc;
        auto &successors = bytecodeBlockInfos_[i].succs;
        for (size_t j = 0; j < successors.size(); j++) {
            if (successors[j] == curPc) {
                continue;
            }
            auto curBlock = endPcToBB[curPc];
            auto succsBlock = startPcToBB[successors[j]];
            curBlock->succs.emplace_back(succsBlock);
            succsBlock->preds.emplace_back(curBlock);
        }
    }

    // try catch block associate
    for (size_t i = 0; i < graph_.size(); i++) {
        const auto pc = graph_[i].start;
        auto it = exception.cbegin();
        for (; it != exception.cend(); it++) {
            if (pc < it->first.first || pc >= it->first.second) { // try block interval
                continue;
            }
            auto catchs = exception[it->first]; // catchs start pc
            for (size_t j = i + 1; j < graph_.size(); j++) {
                if (std::find(catchs.cbegin(), catchs.cend(), graph_[j].start) != catchs.cend()) {
                    graph_[i].catchs.insert(graph_[i].catchs.cbegin(), &graph_[j]);
                    graph_[i].succs.emplace_back(&graph_[j]);
                    graph_[j].preds.emplace_back(&graph_[i]);
                }
            }
        }

        // When there are multiple catch blocks in the current block, the set of catch blocks
        // needs to be sorted to satisfy the order of execution of catch blocks.
        BytecodeRegion& bb = graph_[i];
        bb.SortCatches();
    }

    if (IsLogEnabled()) {
        PrintGraph("Build Basic Block");
    }
    ComputeDominatorTree();
}

void BytecodeCircuitBuilder::ComputeDominatorTree()
{
    // Construct graph backward order
    std::map<size_t, size_t> bbIdToDfsTimestamp; // (basicblock id, dfs order)
    std::unordered_map<size_t, size_t> dfsFatherIdx;
    std::unordered_map<size_t, size_t> bbDfsTimestampToIdx;
    std::vector<size_t> basicBlockList;
    size_t timestamp = 0;
    std::deque<size_t> pendingList;
    std::vector<size_t> visited(graph_.size(), 0);
    auto basicBlockId = graph_[0].id;
    visited[graph_[0].id] = 1;
    pendingList.push_back(basicBlockId);
    while (!pendingList.empty()) {
        size_t curBlockId = pendingList.back();
        pendingList.pop_back();
        basicBlockList.push_back(curBlockId);
        bbIdToDfsTimestamp[curBlockId] = timestamp++;
        for (const auto &succBlock: graph_[curBlockId].succs) {
            if (visited[succBlock->id] == 0) {
                visited[succBlock->id] = 1;
                pendingList.push_back(succBlock->id);
                dfsFatherIdx[succBlock->id] = bbIdToDfsTimestamp[curBlockId];
            }
        }
    }

    for (size_t idx = 0; idx < basicBlockList.size(); idx++) {
        bbDfsTimestampToIdx[basicBlockList[idx]] = idx;
    }
    RemoveDeadRegions(bbIdToDfsTimestamp);

    std::vector<size_t> immDom(basicBlockList.size()); // immediate dominator with dfs order index
    std::vector<size_t> semiDom(basicBlockList.size());
    std::vector<size_t> realImmDom(graph_.size()); // immediate dominator with real index
    std::vector<std::vector<size_t> > semiDomTree(basicBlockList.size());
    {
        std::vector<size_t> parent(basicBlockList.size());
        std::iota(parent.begin(), parent.end(), 0);
        std::vector<size_t> minIdx(basicBlockList.size());
        std::function<size_t(size_t)> unionFind = [&] (size_t idx) -> size_t {
            if (parent[idx] == idx) return idx;
            size_t unionFindSetRoot = unionFind(parent[idx]);
            if (semiDom[minIdx[idx]] > semiDom[minIdx[parent[idx]]]) {
                minIdx[idx] = minIdx[parent[idx]];
            }
            return parent[idx] = unionFindSetRoot;
        };
        auto merge = [&] (size_t fatherIdx, size_t sonIdx) -> void {
            size_t parentFatherIdx = unionFind(fatherIdx);
            size_t parentSonIdx = unionFind(sonIdx);
            parent[parentSonIdx] = parentFatherIdx;
        };
        std::iota(semiDom.begin(), semiDom.end(), 0);
        semiDom[0] = semiDom.size();
        for (size_t idx = basicBlockList.size() - 1; idx >= 1; idx--) {
            for (const auto &preBlock : graph_[basicBlockList[idx]].preds) {
                if (bbDfsTimestampToIdx[preBlock->id] < idx) {
                    semiDom[idx] = std::min(semiDom[idx], bbDfsTimestampToIdx[preBlock->id]);
                } else {
                    unionFind(bbDfsTimestampToIdx[preBlock->id]);
                    semiDom[idx] = std::min(semiDom[idx], semiDom[minIdx[bbDfsTimestampToIdx[preBlock->id]]]);
                }
            }
            for (const auto & succDomIdx : semiDomTree[idx]) {
                unionFind(succDomIdx);
                if (idx == semiDom[minIdx[succDomIdx]]) {
                    immDom[succDomIdx] = idx;
                } else {
                    immDom[succDomIdx] = minIdx[succDomIdx];
                }
            }
            minIdx[idx] = idx;
            merge(dfsFatherIdx[basicBlockList[idx]], idx);
            semiDomTree[semiDom[idx]].push_back(idx);
        }
        for (size_t idx = 1; idx < basicBlockList.size(); idx++) {
            if (immDom[idx] != semiDom[idx]) {
                immDom[idx] = immDom[immDom[idx]];
            }
            realImmDom[basicBlockList[idx]] = basicBlockList[immDom[idx]];
        }
        semiDom[0] = 0;
    }

    if (IsLogEnabled()) {
        PrintGraph("Computed Dom Trees");
    }

    BuildImmediateDominator(realImmDom);
}

void BytecodeCircuitBuilder::BuildImmediateDominator(const std::vector<size_t> &immDom)
{
    graph_[0].iDominator = &graph_[0];
    for (size_t i = 1; i < immDom.size(); i++) {
        auto dominatedBlock = &graph_[i];
        if (dominatedBlock->isDead) {
            continue;
        }
        auto immDomBlock = &graph_[immDom[i]];
        dominatedBlock->iDominator = immDomBlock;
    }

    for (auto &block : graph_) {
        if (block.isDead) {
            continue;
        }
        if (block.iDominator->id != block.id) {
            block.iDominator->immDomBlocks.emplace_back(&block);
        }
    }

    ComputeDomFrontiers(immDom);
    InsertPhi();
    UpdateCFG();
    BuildCircuit();
}

void BytecodeCircuitBuilder::ComputeDomFrontiers(const std::vector<size_t> &immDom)
{
    std::vector<std::set<BytecodeRegion *>> domFrontiers(immDom.size());
    for (auto &bb : graph_) {
        if (bb.isDead) {
            continue;
        }
        if (bb.preds.size() < 2) { // 2: pred num
            continue;
        }
        for (size_t i = 0; i < bb.preds.size(); i++) {
            auto runner = bb.preds[i];
            while (runner->id != immDom[bb.id]) {
                domFrontiers[runner->id].insert(&bb);
                runner = &graph_[immDom[runner->id]];
            }
        }
    }

    for (size_t i = 0; i < domFrontiers.size(); i++) {
        for (auto iter = domFrontiers[i].cbegin(); iter != domFrontiers[i].cend(); iter++) {
            graph_[i].domFrontiers.emplace_back(*iter);
        }
    }
}

void BytecodeCircuitBuilder::RemoveDeadRegions(const std::map<size_t, size_t> &bbIdToDfsTimestamp)
{
    for (auto &block: graph_) {
        std::vector<BytecodeRegion *> newPreds;
        for (auto &bb : block.preds) {
            if (bbIdToDfsTimestamp.count(bb->id)) {
                newPreds.emplace_back(bb);
            }
        }
        block.preds = newPreds;
    }

    for (auto &block : graph_) {
        block.isDead = !bbIdToDfsTimestamp.count(block.id);
        if (block.isDead) {
            block.succs.clear();
        }
    }
}

BytecodeInfo BytecodeCircuitBuilder::GetBytecodeInfo(const uint8_t *pc)
{
    BytecodeInfo info;
    BytecodeInstruction inst(pc);
    auto opcode = inst.GetOpcode();
    info.offset = BytecodeInstruction::Size(opcode);
    info.opcode = static_cast<EcmaOpcode>(opcode);
    info.pcOffset = pc - method_->GetBytecodeArray();
    info.accIn = inst.HasFlag(BytecodeInstruction::Flags::ACC_READ);
    info.accOut = inst.HasFlag(BytecodeInstruction::Flags::ACC_WRITE);
    switch (static_cast<EcmaOpcode>(opcode)) {
        case EcmaOpcode::MOV_V4_V4: {
            uint16_t vdst = READ_INST_4_0();
            uint16_t vsrc = READ_INST_4_1();
            info.vregOut.emplace_back(vdst);
            info.inputs.emplace_back(VirtualRegister(vsrc));
            break;
        }
        case EcmaOpcode::MOV_V8_V8: {
            uint16_t vdst = READ_INST_8_0();
            uint16_t vsrc = READ_INST_8_1();
            info.vregOut.emplace_back(vdst);
            info.inputs.emplace_back(VirtualRegister(vsrc));
            break;
        }
        case EcmaOpcode::MOV_V16_V16: {
            uint16_t vdst = READ_INST_16_0();
            uint16_t vsrc = READ_INST_16_2();
            info.vregOut.emplace_back(vdst);
            info.inputs.emplace_back(VirtualRegister(vsrc));
            break;
        }
        case EcmaOpcode::LDA_STR_ID16: {
            uint16_t stringId = READ_INST_16_0();
            info.inputs.emplace_back(StringId(stringId));
            break;
        }
        case EcmaOpcode::LDA_V8: {
            uint16_t vsrc = READ_INST_8_0();
            info.inputs.emplace_back(VirtualRegister(vsrc));
            break;
        }
        case EcmaOpcode::STA_V8: {
            uint16_t vdst = READ_INST_8_0();
            info.vregOut.emplace_back(vdst);
            break;
        }
        case EcmaOpcode::LDAI_IMM32: {
            info.inputs.emplace_back(Immediate(READ_INST_32_0()));
            break;
        }
        case EcmaOpcode::FLDAI_IMM64: {
            info.inputs.emplace_back(Immediate(READ_INST_64_0()));
            break;
        }
        case EcmaOpcode::CALLARG1_IMM8_V8: {
            uint32_t a0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(a0));
            break;
        }
        case EcmaOpcode::CALLTHIS1_IMM8_V8_V8: {
            uint32_t startReg = READ_INST_8_1();// this
            uint32_t a0 = READ_INST_8_2();
            info.inputs.emplace_back(VirtualRegister(startReg));
            info.inputs.emplace_back(VirtualRegister(a0));
            break;
        }
        case EcmaOpcode::CALLARGS2_IMM8_V8_V8: {
            uint32_t a0 = READ_INST_8_1();
            uint32_t a1 = READ_INST_8_2();
            info.inputs.emplace_back(VirtualRegister(a0));
            info.inputs.emplace_back(VirtualRegister(a1));
            break;
        }
        case EcmaOpcode::CALLARGS3_IMM8_V8_V8_V8: {
            uint32_t a0 = READ_INST_8_1();
            uint32_t a1 = READ_INST_8_2();
            uint32_t a2 = READ_INST_8_3();
            info.inputs.emplace_back(VirtualRegister(a0));
            info.inputs.emplace_back(VirtualRegister(a1));
            info.inputs.emplace_back(VirtualRegister(a2));
            break;
        }
        case EcmaOpcode::CALLTHISRANGE_IMM8_IMM8_V8: {
            uint32_t actualNumArgs = READ_INST_8_1();
            uint32_t startReg = READ_INST_8_2();
            info.inputs.emplace_back(VirtualRegister(startReg));
            for (size_t i = 1; i <= actualNumArgs; i++) {
                info.inputs.emplace_back(VirtualRegister(startReg + i));
            }
            break;
        }
        case EcmaOpcode::WIDE_CALLTHISRANGE_PREF_IMM16_V8: {
            uint32_t actualNumArgs = READ_INST_16_1();
            uint32_t startReg = READ_INST_8_3();
            info.inputs.emplace_back(VirtualRegister(startReg));
            for (size_t i = 1; i <= actualNumArgs; i++) {
                info.inputs.emplace_back(VirtualRegister(startReg + i));
            }
            break;
        }
        case EcmaOpcode::CALLTHIS0_IMM8_V8: {
            int32_t startReg = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(startReg));
            break;
        }
        case EcmaOpcode::CALLTHIS2_IMM8_V8_V8_V8: {
            int32_t startReg = READ_INST_8_1();
            uint32_t a0 = READ_INST_8_2();
            uint32_t a1 = READ_INST_8_3();
            info.inputs.emplace_back(VirtualRegister(startReg));
            info.inputs.emplace_back(VirtualRegister(a0));
            info.inputs.emplace_back(VirtualRegister(a1));
            break;
        }
        case EcmaOpcode::CALLTHIS3_IMM8_V8_V8_V8_V8: {
            int32_t startReg = READ_INST_8_1();
            uint32_t a0 = READ_INST_8_2();
            uint32_t a1 = READ_INST_8_3();
            uint32_t a2 = READ_INST_8_4();
            info.inputs.emplace_back(VirtualRegister(startReg));
            info.inputs.emplace_back(VirtualRegister(a0));
            info.inputs.emplace_back(VirtualRegister(a1));
            info.inputs.emplace_back(VirtualRegister(a2));
            break;
        }

        case EcmaOpcode::APPLY_IMM8_V8_V8: {
            uint16_t v0 = READ_INST_8_1();
            uint16_t v1 = READ_INST_8_2();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.inputs.emplace_back(VirtualRegister(v1));
            break;
        }
        case EcmaOpcode::CALLRANGE_IMM8_IMM8_V8: {
            int32_t actualNumArgs = READ_INST_8_1();
            int32_t startReg = READ_INST_8_2();
            for (int i = 0; i < actualNumArgs; i++) {
                info.inputs.emplace_back(VirtualRegister(startReg + i));
            }
            break;
        }
        case EcmaOpcode::WIDE_CALLRANGE_PREF_IMM16_V8: {
            int32_t actualNumArgs = READ_INST_16_1();
            int32_t startReg = READ_INST_8_3();
            for (int i = 0; i < actualNumArgs; i++) {
                info.inputs.emplace_back(VirtualRegister(startReg + i));
            }
            break;
        }
        case EcmaOpcode::RETURNUNDEFINED:
            info.accIn = true;
            break;
        case EcmaOpcode::THROW_CONSTASSIGNMENT_PREF_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::THROW_IFNOTOBJECT_PREF_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::THROW_UNDEFINEDIFHOLE_PREF_V8_V8: {
            uint16_t v0 = READ_INST_8_1();
            uint16_t v1 = READ_INST_8_2();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.inputs.emplace_back(VirtualRegister(v1));
            break;
        }
        case EcmaOpcode::THROW_IFSUPERNOTCORRECTCALL_PREF_IMM8: {
            uint8_t imm = READ_INST_8_1();
            info.inputs.emplace_back(Immediate(imm));
            break;
        }
        case EcmaOpcode::THROW_IFSUPERNOTCORRECTCALL_PREF_IMM16: {
            uint16_t imm = READ_INST_16_1();
            info.inputs.emplace_back(Immediate(imm));
            break;
        }
        case EcmaOpcode::CLOSEITERATOR_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::ADD2_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.deopt = true;
            break;
        }
        case EcmaOpcode::SUB2_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.deopt = true;
            break;
        }
        case EcmaOpcode::MUL2_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.deopt = true;
            break;
        }
        case EcmaOpcode::DIV2_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.deopt = true;
            break;
        }
        case EcmaOpcode::MOD2_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.deopt = true;
            break;
        }
        case EcmaOpcode::EQ_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.deopt = true;
            break;
        }
        case EcmaOpcode::NOTEQ_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.deopt = true;
            break;
        }
        case EcmaOpcode::LESS_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.deopt = true;
            break;
        }
        case EcmaOpcode::LESSEQ_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.deopt = true;
            break;
        }
        case EcmaOpcode::GREATER_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.deopt = true;
            break;
        }
        case EcmaOpcode::GREATEREQ_IMM8_V8: {
            uint16_t vs = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(vs));
            info.deopt = true;
            break;
        }
        case EcmaOpcode::SHL2_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.deopt = true;
            break;
        }
        case EcmaOpcode::SHR2_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.deopt = true;
            break;
        }
        case EcmaOpcode::ASHR2_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.deopt = true;
            break;
        }
        case EcmaOpcode::AND2_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.deopt = true;
            break;
        }
        case EcmaOpcode::OR2_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.deopt = true;
            break;
        }
        case EcmaOpcode::XOR2_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.deopt = true;
            break;
        }
        case EcmaOpcode::EXP_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.deopt = true;
            break;
        }
        case EcmaOpcode::ISIN_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::INSTANCEOF_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::STRICTNOTEQ_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::STRICTEQ_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::DELOBJPROP_V8: {
            uint16_t v0 = READ_INST_8_0();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::DEFINEFUNC_IMM8_ID16_IMM8: {
            uint16_t methodId = READ_INST_16_1();
            uint16_t length = READ_INST_8_3();
            info.inputs.emplace_back(MethodId(methodId));
            info.inputs.emplace_back(Immediate(length));
            break;
        }
        case EcmaOpcode::DEFINEFUNC_IMM16_ID16_IMM8: {
            uint16_t methodId = READ_INST_16_2();
            uint16_t length = READ_INST_8_4();
            info.inputs.emplace_back(MethodId(methodId));
            info.inputs.emplace_back(Immediate(length));
            break;
        }
        case EcmaOpcode::DEFINEMETHOD_IMM8_ID16_IMM8: {
            uint16_t methodId = READ_INST_16_1();
            uint16_t length = READ_INST_8_3();
            info.inputs.emplace_back(MethodId(methodId));
            info.inputs.emplace_back(Immediate(length));
            break;
        }
        case EcmaOpcode::DEFINEMETHOD_IMM16_ID16_IMM8: {
            uint16_t methodId = READ_INST_16_2();
            uint16_t length = READ_INST_8_4();
            info.inputs.emplace_back(MethodId(methodId));
            info.inputs.emplace_back(Immediate(length));
            break;
        }
        case EcmaOpcode::NEWOBJRANGE_IMM8_IMM8_V8: {
            uint16_t numArgs = READ_INST_8_1();
            uint16_t firstArgRegIdx = READ_INST_8_2();
            for (uint16_t i = 0; i < numArgs; ++i) {
                info.inputs.emplace_back(VirtualRegister(firstArgRegIdx + i));
            }
            break;
        }
        case EcmaOpcode::NEWOBJRANGE_IMM16_IMM8_V8: {
            uint16_t numArgs = READ_INST_8_2();
            uint16_t firstArgRegIdx = READ_INST_8_3();
            for (uint16_t i = 0; i < numArgs; ++i) {
                info.inputs.emplace_back(VirtualRegister(firstArgRegIdx + i));
            }
            break;
        }
        case EcmaOpcode::WIDE_NEWOBJRANGE_PREF_IMM16_V8: {
            uint16_t numArgs = READ_INST_16_1();
            uint16_t firstArgRegIdx = READ_INST_8_3();
            for (uint16_t i = 0; i < numArgs; ++i) {
                info.inputs.emplace_back(VirtualRegister(firstArgRegIdx + i));
            }
            break;
        }
        case EcmaOpcode::LDLEXVAR_IMM4_IMM4: {
            uint16_t level = READ_INST_4_0();
            uint16_t slot = READ_INST_4_1();
            info.inputs.emplace_back(Immediate(level));
            info.inputs.emplace_back(Immediate(slot));
            break;
        }
        case EcmaOpcode::LDLEXVAR_IMM8_IMM8: {
            uint16_t level = READ_INST_8_0();
            uint16_t slot = READ_INST_8_1();
            info.inputs.emplace_back(Immediate(level));
            info.inputs.emplace_back(Immediate(slot));
            break;
        }
        case EcmaOpcode::WIDE_LDLEXVAR_PREF_IMM16_IMM16: {
            uint16_t level = READ_INST_16_1();
            uint16_t slot = READ_INST_16_3();
            info.inputs.emplace_back(Immediate(level));
            info.inputs.emplace_back(Immediate(slot));
            break;
        }
        case EcmaOpcode::STLEXVAR_IMM4_IMM4: {
            uint16_t level = READ_INST_4_0();
            uint16_t slot = READ_INST_4_1();
            info.inputs.emplace_back(Immediate(level));
            info.inputs.emplace_back(Immediate(slot));
            break;
        }
        case EcmaOpcode::STLEXVAR_IMM8_IMM8: {
            uint16_t level = READ_INST_8_0();
            uint16_t slot = READ_INST_8_1();
            info.inputs.emplace_back(Immediate(level));
            info.inputs.emplace_back(Immediate(slot));
            break;
        }
        case EcmaOpcode::WIDE_STLEXVAR_PREF_IMM16_IMM16: {
            uint16_t level = READ_INST_16_1();
            uint16_t slot = READ_INST_16_3();
            info.inputs.emplace_back(Immediate(level));
            info.inputs.emplace_back(Immediate(slot));
            break;
        }
        case EcmaOpcode::NEWLEXENV_IMM8: {
            uint8_t numVars = READ_INST_8_0();
            info.inputs.emplace_back(Immediate(numVars));
            break;
        }
        case EcmaOpcode::WIDE_NEWLEXENV_PREF_IMM16: {
            uint16_t numVars = READ_INST_16_1();
            info.inputs.emplace_back(Immediate(numVars));
            break;
        }
        case EcmaOpcode::NEWLEXENVWITHNAME_IMM8_ID16: {
            uint16_t numVars = READ_INST_8_0();
            uint16_t scopeId = READ_INST_16_1();
            info.inputs.emplace_back(Immediate(numVars));
            info.inputs.emplace_back(Immediate(scopeId));
            break;
        }
        case EcmaOpcode::WIDE_NEWLEXENVWITHNAME_PREF_IMM16_ID16: {
            uint16_t numVars = READ_INST_16_1();
            uint16_t scopeId = READ_INST_16_3();
            info.inputs.emplace_back(Immediate(numVars));
            info.inputs.emplace_back(Immediate(scopeId));
            break;
        }
        case EcmaOpcode::CREATEITERRESULTOBJ_V8_V8: {
            uint16_t v0 = READ_INST_8_0();
            uint16_t v1 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.inputs.emplace_back(VirtualRegister(v1));
            break;
        }
        case EcmaOpcode::SUSPENDGENERATOR_V8: {
            uint16_t v0 = READ_INST_8_0();
            uint32_t offset = pc - method_->GetBytecodeArray();
            info.inputs.emplace_back(Immediate(offset)); // Save the pc offset when suspend
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::ASYNCFUNCTIONAWAITUNCAUGHT_V8: {
            uint16_t v0 = READ_INST_8_0();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::ASYNCFUNCTIONRESOLVE_V8: {
            uint16_t v0 = READ_INST_8_0();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::ASYNCFUNCTIONREJECT_V8: {
            uint16_t v0 = READ_INST_8_0();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::CLOSEITERATOR_IMM16_V8: {
            uint16_t v0 = READ_INST_8_2();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::NEWOBJAPPLY_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::NEWOBJAPPLY_IMM16_V8: {
            uint16_t v0 = READ_INST_8_2();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::STOWNBYNAME_IMM8_ID16_V8: {
            uint16_t stringId = READ_INST_16_1();
            uint32_t v0 = READ_INST_8_3();
            info.inputs.emplace_back(StringId(stringId));
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::STOWNBYNAME_IMM16_ID16_V8: {
            uint16_t stringId = READ_INST_16_2();
            uint32_t v0 = READ_INST_8_4();
            info.inputs.emplace_back(StringId(stringId));
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::CREATEREGEXPWITHLITERAL_IMM8_ID16_IMM8: {
            uint16_t stringId = READ_INST_16_1();
            uint8_t flags = READ_INST_8_3();
            info.inputs.emplace_back(StringId(stringId));
            info.inputs.emplace_back(Immediate(flags));
            break;
        }
        case EcmaOpcode::CREATEREGEXPWITHLITERAL_IMM16_ID16_IMM8: {
            uint16_t stringId = READ_INST_16_2();
            uint8_t flags = READ_INST_8_4();
            info.inputs.emplace_back(StringId(stringId));
            info.inputs.emplace_back(Immediate(flags));
            break;
        }
        case EcmaOpcode::GETNEXTPROPNAME_V8: {
            uint16_t v0 = READ_INST_8_0();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::CREATEOBJECTWITHBUFFER_IMM8_ID16: {
            uint16_t imm = READ_INST_16_1();
            info.inputs.emplace_back(Immediate(imm));
            break;
        }
        case EcmaOpcode::CREATEOBJECTWITHBUFFER_IMM16_ID16: {
            uint16_t imm = READ_INST_16_2();
            info.inputs.emplace_back(Immediate(imm));
            break;
        }
        case EcmaOpcode::SETOBJECTWITHPROTO_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::SETOBJECTWITHPROTO_IMM16_V8: {
            uint16_t v0 = READ_INST_8_2();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::CREATEARRAYWITHBUFFER_IMM8_ID16: {
            uint16_t imm = READ_INST_16_1();
            info.inputs.emplace_back(Immediate(imm));
            break;
        }
        case EcmaOpcode::CREATEARRAYWITHBUFFER_IMM16_ID16: {
            uint16_t imm = READ_INST_16_2();
            info.inputs.emplace_back(Immediate(imm));
            break;
        }
        case EcmaOpcode::GETMODULENAMESPACE_IMM8: {
            int32_t index = READ_INST_8_0();
            info.inputs.emplace_back(Immediate(index));
            break;
        }
        case EcmaOpcode::WIDE_GETMODULENAMESPACE_PREF_IMM16: {
            int32_t index = READ_INST_16_1();
            info.inputs.emplace_back(Immediate(index));
            break;
        }
        case EcmaOpcode::STMODULEVAR_IMM8: {
            int32_t index = READ_INST_8_0();
            info.inputs.emplace_back(Immediate(index));
            break;
        }
        case EcmaOpcode::WIDE_STMODULEVAR_PREF_IMM16: {
            int32_t index = READ_INST_16_1();
            info.inputs.emplace_back(Immediate(index));
            break;
        }
        case EcmaOpcode::LDLOCALMODULEVAR_IMM8: {
            int32_t index = READ_INST_8_0();
            info.inputs.emplace_back(Immediate(index));
            break;
        }
        case EcmaOpcode::WIDE_LDLOCALMODULEVAR_PREF_IMM16: {
            int32_t index = READ_INST_16_1();
            info.inputs.emplace_back(Immediate(index));
            break;
        }
        case EcmaOpcode::LDEXTERNALMODULEVAR_IMM8: {
            uint16_t stringId = READ_INST_8_0();
            info.inputs.emplace_back(StringId(stringId));
            break;
        }
        case EcmaOpcode::WIDE_LDEXTERNALMODULEVAR_PREF_IMM16: {
            uint16_t stringId = READ_INST_16_1();
            info.inputs.emplace_back(StringId(stringId));
            break;
        }
        case EcmaOpcode::STCONSTTOGLOBALRECORD_IMM16_ID16: {
            uint16_t stringId = READ_INST_16_2();
            info.inputs.emplace_back(StringId(stringId));
            break;
        }
        case EcmaOpcode::GETTEMPLATEOBJECT_IMM8:
        case EcmaOpcode::GETTEMPLATEOBJECT_IMM16:
            break;
        case EcmaOpcode::COPYDATAPROPERTIES_V8: {
            uint16_t v0 = READ_INST_8_0();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::STOWNBYINDEX_IMM8_V8_IMM16: {
            uint8_t v0 = READ_INST_8_1();
            uint16_t index = READ_INST_16_2();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.inputs.emplace_back(Immediate(index));
            break;
        }
        case EcmaOpcode::STOWNBYINDEX_IMM16_V8_IMM16: {
            uint8_t v0 = READ_INST_8_2();
            uint16_t index = READ_INST_16_3();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.inputs.emplace_back(Immediate(index));
            break;
        }
        case EcmaOpcode::WIDE_STOWNBYINDEX_PREF_V8_IMM32: {
            uint32_t v0 = READ_INST_8_1();
            uint32_t index = READ_INST_32_2();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.inputs.emplace_back(Immediate(index));
            break;
        }
        case EcmaOpcode::STOWNBYVALUE_IMM8_V8_V8: {
            uint32_t v0 = READ_INST_8_1();
            uint32_t v1 = READ_INST_8_2();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.inputs.emplace_back(VirtualRegister(v1));
            break;
        }
        case EcmaOpcode::STOWNBYVALUE_IMM16_V8_V8: {
            uint32_t v0 = READ_INST_8_2();
            uint32_t v1 = READ_INST_8_3();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.inputs.emplace_back(VirtualRegister(v1));
            break;
        }
        case EcmaOpcode::CREATEOBJECTWITHEXCLUDEDKEYS_IMM8_V8_V8: {
            uint8_t numKeys = READ_INST_8_0();
            uint16_t v0 = READ_INST_8_1();
            uint16_t firstArgRegIdx = READ_INST_8_2();
            info.inputs.emplace_back(Immediate(numKeys));
            info.inputs.emplace_back(VirtualRegister(v0));
            info.inputs.emplace_back(Immediate(firstArgRegIdx));
            break;
        }
        case EcmaOpcode::WIDE_CREATEOBJECTWITHEXCLUDEDKEYS_PREF_IMM16_V8_V8: {
            uint16_t numKeys = READ_INST_16_1();
            uint16_t v0 = READ_INST_8_3();
            uint16_t firstArgRegIdx = READ_INST_8_4();
            info.inputs.emplace_back(Immediate(numKeys));
            info.inputs.emplace_back(VirtualRegister(v0));
            info.inputs.emplace_back(Immediate(firstArgRegIdx));
            break;
        }
        case EcmaOpcode::COPYRESTARGS_IMM8: {
            uint16_t restIdx = READ_INST_8_0();
            info.inputs.emplace_back(Immediate(restIdx));
            break;
        }
        case EcmaOpcode::WIDE_COPYRESTARGS_PREF_IMM16: {
            uint16_t restIdx = READ_INST_16_1();
            info.inputs.emplace_back(Immediate(restIdx));
            break;
        }
        case EcmaOpcode::DEFINEGETTERSETTERBYVALUE_V8_V8_V8_V8: {
            uint16_t v0 = READ_INST_8_0();
            uint16_t v1 = READ_INST_8_1();
            uint16_t v2 = READ_INST_8_2();
            uint16_t v3 = READ_INST_8_3();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.inputs.emplace_back(VirtualRegister(v1));
            info.inputs.emplace_back(VirtualRegister(v2));
            info.inputs.emplace_back(VirtualRegister(v3));
            break;
        }
        case EcmaOpcode::LDOBJBYINDEX_IMM8_IMM16: {
            uint32_t idx = READ_INST_16_1();
            info.inputs.emplace_back(Immediate(idx));
            break;
        }
        case EcmaOpcode::LDOBJBYINDEX_IMM16_IMM16: {
            uint32_t idx = READ_INST_16_2();
            info.inputs.emplace_back(Immediate(idx));
            break;
        }
        case EcmaOpcode::WIDE_LDOBJBYINDEX_PREF_IMM32: {
            uint32_t idx = READ_INST_32_1();
            info.inputs.emplace_back(Immediate(idx));
            break;
        }
        case EcmaOpcode::STOBJBYINDEX_IMM8_V8_IMM16: {
            uint8_t v0 = READ_INST_8_1();
            uint16_t index = READ_INST_16_2();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.inputs.emplace_back(Immediate(index));
            break;
        }
        case EcmaOpcode::STOBJBYINDEX_IMM16_V8_IMM16: {
            uint8_t v0 = READ_INST_8_2();
            uint16_t index = READ_INST_16_3();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.inputs.emplace_back(Immediate(index));
            break;
        }
        case EcmaOpcode::WIDE_STOBJBYINDEX_PREF_V8_IMM32: {
            uint8_t v0 = READ_INST_8_1();
            uint32_t index = READ_INST_32_2();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.inputs.emplace_back(Immediate(index));
            break;
        }
        case EcmaOpcode::LDOBJBYVALUE_IMM8_V8: {
            uint32_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::LDOBJBYVALUE_IMM16_V8: {
            uint32_t v0 = READ_INST_8_2();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::STOBJBYVALUE_IMM8_V8_V8: {
            uint32_t v0 = READ_INST_8_1();
            uint32_t v1 = READ_INST_8_2();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.inputs.emplace_back(VirtualRegister(v1));
            break;
        }
        case EcmaOpcode::STOBJBYVALUE_IMM16_V8_V8: {
            uint32_t v0 = READ_INST_8_2();
            uint32_t v1 = READ_INST_8_3();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.inputs.emplace_back(VirtualRegister(v1));
            break;
        }
        case EcmaOpcode::LDSUPERBYVALUE_IMM8_V8: {
            uint32_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::LDSUPERBYVALUE_IMM16_V8: {
            uint32_t v0 = READ_INST_8_2();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::STSUPERBYVALUE_IMM8_V8_V8: {
            uint32_t v0 = READ_INST_8_1();
            uint32_t v1 = READ_INST_8_2();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.inputs.emplace_back(VirtualRegister(v1));
            break;
        }
        case EcmaOpcode::STSUPERBYVALUE_IMM16_V8_V8: {
            uint32_t v0 = READ_INST_8_2();
            uint32_t v1 = READ_INST_8_3();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.inputs.emplace_back(VirtualRegister(v1));
            break;
        }
        case EcmaOpcode::TRYLDGLOBALBYNAME_IMM8_ID16: {
            uint16_t stringId = READ_INST_16_1();
            info.inputs.emplace_back(StringId(stringId));
            break;
        }
        case EcmaOpcode::TRYLDGLOBALBYNAME_IMM16_ID16: {
            uint16_t stringId = READ_INST_16_2();
            info.inputs.emplace_back(StringId(stringId));
            break;
        }
        case EcmaOpcode::TRYSTGLOBALBYNAME_IMM8_ID16: {
            uint16_t stringId = READ_INST_16_1();
            info.inputs.emplace_back(StringId(stringId));
            break;
        }
        case EcmaOpcode::TRYSTGLOBALBYNAME_IMM16_ID16: {
            uint16_t stringId = READ_INST_16_2();
            info.inputs.emplace_back(StringId(stringId));
            break;
        }
        case EcmaOpcode::STTOGLOBALRECORD_IMM16_ID16: {
            uint16_t stringId = READ_INST_16_2();
            info.inputs.emplace_back(StringId(stringId));
            break;
        }
        case EcmaOpcode::STOWNBYVALUEWITHNAMESET_IMM8_V8_V8: {
            uint32_t v0 = READ_INST_8_1();
            uint32_t v1 = READ_INST_8_2();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.inputs.emplace_back(VirtualRegister(v1));
            break;
        }
        case EcmaOpcode::STOWNBYVALUEWITHNAMESET_IMM16_V8_V8: {
            uint32_t v0 = READ_INST_8_2();
            uint32_t v1 = READ_INST_8_3();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.inputs.emplace_back(VirtualRegister(v1));
            break;
        }
        case EcmaOpcode::STOWNBYNAMEWITHNAMESET_IMM8_ID16_V8: {
            uint16_t stringId = READ_INST_16_1();
            uint32_t v0 = READ_INST_8_3();
            info.inputs.emplace_back(StringId(stringId));
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::STOWNBYNAMEWITHNAMESET_IMM16_ID16_V8: {
            uint16_t stringId = READ_INST_16_2();
            uint32_t v0 = READ_INST_8_4();
            info.inputs.emplace_back(StringId(stringId));
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::STTHISBYVALUE_IMM8_V8: {
            uint32_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::STTHISBYVALUE_IMM16_V8: {
            uint32_t v0 = READ_INST_8_2();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        // not implement
        case EcmaOpcode::JSTRICTEQZ_IMM8:
        case EcmaOpcode::JSTRICTEQZ_IMM16:
        case EcmaOpcode::JNSTRICTEQZ_IMM8:
        case EcmaOpcode::JNSTRICTEQZ_IMM16:
        case EcmaOpcode::JEQNULL_IMM8:
        case EcmaOpcode::JEQNULL_IMM16:
        case EcmaOpcode::JNENULL_IMM8:
        case EcmaOpcode::JNENULL_IMM16:
        case EcmaOpcode::JSTRICTEQNULL_IMM8:
        case EcmaOpcode::JSTRICTEQNULL_IMM16:
        case EcmaOpcode::JNSTRICTEQNULL_IMM8:
        case EcmaOpcode::JNSTRICTEQNULL_IMM16:
        case EcmaOpcode::JEQUNDEFINED_IMM8:
        case EcmaOpcode::JEQUNDEFINED_IMM16:
        case EcmaOpcode::JNEUNDEFINED_IMM8:
        case EcmaOpcode::JNEUNDEFINED_IMM16:
        case EcmaOpcode::JSTRICTEQUNDEFINED_IMM8:
        case EcmaOpcode::JSTRICTEQUNDEFINED_IMM16:
        case EcmaOpcode::JNSTRICTEQUNDEFINED_IMM8:
        case EcmaOpcode::JNSTRICTEQUNDEFINED_IMM16:
        case EcmaOpcode::JEQ_V8_IMM8:
        case EcmaOpcode::JEQ_V8_IMM16:
        case EcmaOpcode::JNE_V8_IMM8:
        case EcmaOpcode::JNE_V8_IMM16:
        case EcmaOpcode::JSTRICTEQ_V8_IMM8:
        case EcmaOpcode::JSTRICTEQ_V8_IMM16:
        case EcmaOpcode::JNSTRICTEQ_V8_IMM8:
        case EcmaOpcode::JNSTRICTEQ_V8_IMM16:
        case EcmaOpcode::LDTHIS:
            break;
        case EcmaOpcode::LDTHISBYNAME_IMM8_ID16:
        case EcmaOpcode::LDTHISBYNAME_IMM16_ID16: {
            uint16_t stringId = READ_INST_16_2();
            info.inputs.emplace_back(StringId(stringId));
            break;
        }
        case EcmaOpcode::STTHISBYNAME_IMM8_ID16:
        case EcmaOpcode::STTHISBYNAME_IMM16_ID16: {
            uint16_t stringId = READ_INST_16_2();
            info.inputs.emplace_back(StringId(stringId));
            break;
        }
        case EcmaOpcode::LDGLOBALVAR_IMM16_ID16: {
            uint16_t stringId = READ_INST_16_2();
            info.inputs.emplace_back(StringId(stringId));
            break;
        }
        case EcmaOpcode::LDOBJBYNAME_IMM8_ID16: {
            uint16_t stringId = READ_INST_16_1();
            info.inputs.emplace_back(StringId(stringId));
            break;
        }
        case EcmaOpcode::LDOBJBYNAME_IMM16_ID16: {
            uint16_t stringId = READ_INST_16_2();
            info.inputs.emplace_back(StringId(stringId));
            break;
        }
        case EcmaOpcode::STOBJBYNAME_IMM8_ID16_V8: {
            uint16_t stringId = READ_INST_16_1();
            uint32_t v0 = READ_INST_8_3();
            info.inputs.emplace_back(StringId(stringId));
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::STOBJBYNAME_IMM16_ID16_V8: {
            uint16_t stringId = READ_INST_16_2();
            uint32_t v0 = READ_INST_8_4();
            info.inputs.emplace_back(StringId(stringId));
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::LDSUPERBYNAME_IMM8_ID16: {
            uint16_t stringId = READ_INST_16_1();
            info.inputs.emplace_back(StringId(stringId));
            break;
        }
        case EcmaOpcode::LDSUPERBYNAME_IMM16_ID16: {
            uint16_t stringId = READ_INST_16_2();
            info.inputs.emplace_back(StringId(stringId));
            break;
        }
        case EcmaOpcode::STSUPERBYNAME_IMM8_ID16_V8: {
            uint16_t stringId = READ_INST_16_1();
            uint32_t v0 = READ_INST_8_3();
            info.inputs.emplace_back(StringId(stringId));
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::STSUPERBYNAME_IMM16_ID16_V8: {
            uint16_t stringId = READ_INST_16_2();
            uint32_t v0 = READ_INST_8_4();
            info.inputs.emplace_back(StringId(stringId));
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::STGLOBALVAR_IMM16_ID16: {
            uint32_t stringId = READ_INST_16_2();
            info.inputs.emplace_back(StringId(stringId));
            break;
        }
        case EcmaOpcode::CREATEGENERATOROBJ_V8: {
            uint16_t v0 = READ_INST_8_0();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::CREATEASYNCGENERATOROBJ_V8: {
            uint16_t v0 = READ_INST_8_0();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::ASYNCGENERATORRESOLVE_V8_V8_V8: {
            uint16_t v0 = READ_INST_8_0();
            uint16_t v1 = READ_INST_8_1();
            uint16_t v2 = READ_INST_8_2();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.inputs.emplace_back(VirtualRegister(v1));
            info.inputs.emplace_back(VirtualRegister(v2));
            break;
        }
        case EcmaOpcode::ASYNCGENERATORREJECT_V8: {
            uint16_t v0 = READ_INST_8_0();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::STARRAYSPREAD_V8_V8: {
            uint16_t v0 = READ_INST_8_0();
            uint16_t v1 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            info.inputs.emplace_back(VirtualRegister(v1));
            break;
        }
        case EcmaOpcode::DEFINECLASSWITHBUFFER_IMM8_ID16_ID16_IMM16_V8: {
            uint16_t methodId = READ_INST_16_1();
            uint16_t literaId = READ_INST_16_3();
            uint16_t length = READ_INST_16_5();
            uint16_t v0 = READ_INST_8_7();
            info.inputs.emplace_back(MethodId(methodId));
            info.inputs.emplace_back(Immediate(literaId));
            info.inputs.emplace_back(Immediate(length));
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::DEFINECLASSWITHBUFFER_IMM16_ID16_ID16_IMM16_V8: {
            uint16_t methodId = READ_INST_16_2();
            uint16_t literaId = READ_INST_16_4();
            uint16_t length = READ_INST_16_6();
            uint16_t v0 = READ_INST_8_8();
            info.inputs.emplace_back(MethodId(methodId));
            info.inputs.emplace_back(Immediate(literaId));
            info.inputs.emplace_back(Immediate(length));
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::LDFUNCTION: {
            break;
        }
        case EcmaOpcode::LDBIGINT_ID16: {
            uint32_t stringId = READ_INST_16_0();
            info.inputs.emplace_back(StringId(stringId));
            break;
        }
        case EcmaOpcode::DYNAMICIMPORT: {
            break;
        }
        case EcmaOpcode::SUPERCALLTHISRANGE_IMM8_IMM8_V8:
        case EcmaOpcode::SUPERCALLARROWRANGE_IMM8_IMM8_V8: {
            uint16_t range = READ_INST_8_1();
            uint16_t v0 = READ_INST_8_2();
            for (size_t i = 0; i < range; i++) {
                info.inputs.emplace_back(VirtualRegister(v0 + i));
            }
            break;
        }
        case EcmaOpcode::WIDE_SUPERCALLTHISRANGE_PREF_IMM16_V8:
        case EcmaOpcode::WIDE_SUPERCALLARROWRANGE_PREF_IMM16_V8: {
            uint16_t range = READ_INST_16_1();
            uint16_t v0 = READ_INST_8_3();
            for (size_t i = 0; i < range; i++) {
                info.inputs.emplace_back(VirtualRegister(v0 + i));
            }
            break;
        }
        case EcmaOpcode::SUPERCALLSPREAD_IMM8_V8: {
            uint16_t v0 = READ_INST_8_1();
            info.inputs.emplace_back(VirtualRegister(v0));
            break;
        }
        case EcmaOpcode::TONUMERIC_IMM8:
        case EcmaOpcode::NEG_IMM8:
        case EcmaOpcode::NOT_IMM8:
        case EcmaOpcode::INC_IMM8:
        case EcmaOpcode::DEC_IMM8: {
            info.deopt = true;
            break;
        }
        case EcmaOpcode::JMP_IMM8:
        case EcmaOpcode::JMP_IMM16:
        case EcmaOpcode::JMP_IMM32:
        case EcmaOpcode::JEQZ_IMM8:
        case EcmaOpcode::JEQZ_IMM16:
        case EcmaOpcode::JEQZ_IMM32:
        case EcmaOpcode::JNEZ_IMM8:
        case EcmaOpcode::JNEZ_IMM16:
        case EcmaOpcode::JNEZ_IMM32:
        case EcmaOpcode::RETURN:
        case EcmaOpcode::LDNAN:
        case EcmaOpcode::LDINFINITY:
        case EcmaOpcode::LDNEWTARGET:
        case EcmaOpcode::LDUNDEFINED:
        case EcmaOpcode::LDNULL:
        case EcmaOpcode::LDSYMBOL:
        case EcmaOpcode::LDGLOBAL:
        case EcmaOpcode::LDTRUE:
        case EcmaOpcode::LDFALSE:
        case EcmaOpcode::LDHOLE:
        case EcmaOpcode::CALLARG0_IMM8:
        case EcmaOpcode::POPLEXENV:
        case EcmaOpcode::GETUNMAPPEDARGS:
        case EcmaOpcode::ASYNCFUNCTIONENTER:
        case EcmaOpcode::TYPEOF_IMM8:
        case EcmaOpcode::TYPEOF_IMM16:
        case EcmaOpcode::TONUMBER_IMM8:
        case EcmaOpcode::THROW_PREF_NONE:
        case EcmaOpcode::GETPROPITERATOR:
        case EcmaOpcode::RESUMEGENERATOR:
        case EcmaOpcode::GETRESUMEMODE:
        case EcmaOpcode::CREATEEMPTYARRAY_IMM8:
        case EcmaOpcode::CREATEEMPTYARRAY_IMM16:
        case EcmaOpcode::CREATEEMPTYOBJECT:
        case EcmaOpcode::DEBUGGER:
        case EcmaOpcode::ISTRUE:
        case EcmaOpcode::ISFALSE:
        case EcmaOpcode::NOP:
        case EcmaOpcode::GETITERATOR_IMM8:
        case EcmaOpcode::GETITERATOR_IMM16:
        case EcmaOpcode::THROW_NOTEXISTS_PREF_NONE:
        case EcmaOpcode::THROW_PATTERNNONCOERCIBLE_PREF_NONE:
        case EcmaOpcode::THROW_DELETESUPERPROPERTY_PREF_NONE:
        case EcmaOpcode::LDTHISBYVALUE_IMM8:
        case EcmaOpcode::LDTHISBYVALUE_IMM16:
            break;
        case EcmaOpcode::WIDE_LDPATCHVAR_PREF_IMM16: {
            uint16_t index = READ_INST_16_1();
            info.inputs.emplace_back(Immediate(index));
            break;
        }
        case EcmaOpcode::WIDE_STPATCHVAR_PREF_IMM16: {
            uint16_t index = READ_INST_16_1();
            info.inputs.emplace_back(Immediate(index));
            break;
        }
        default: {
            LOG_COMPILER(FATAL) << "Error bytecode: " << static_cast<uint16_t>(opcode);
            UNREACHABLE();
            break;
        }
    }
    return info;
}

void BytecodeCircuitBuilder::InsertPhi()
{
    std::map<uint16_t, std::set<size_t>> defsitesInfo; // <vreg, bbs>
    for (auto &bb : graph_) {
        if (bb.isDead) {
            continue;
        }
        EnumerateBlock(bb, [this, &defsitesInfo, &bb]
        ([[maybe_unused]]uint8_t * pc, BytecodeInfo &bytecodeInfo) -> bool {
            if (bytecodeInfo.IsBc(EcmaOpcode::RESUMEGENERATOR)) {
                auto numVRegs = method_->GetNumberVRegs();
                for (size_t i = 0; i < numVRegs; i++) {
                    bytecodeInfo.vregOut.emplace_back(i);
                }
            }
            for (const auto &vreg: bytecodeInfo.vregOut) {
                defsitesInfo[vreg].insert(bb.id);
            }
            return true;
        });
    }

    // handle phi generated from multiple control flow in the same source block
    InsertExceptionPhi(defsitesInfo);

    for (const auto&[variable, defsites] : defsitesInfo) {
        std::queue<uint16_t> workList;
        for (auto blockId: defsites) {
            workList.push(blockId);
        }
        while (!workList.empty()) {
            auto currentId = workList.front();
            workList.pop();
            for (auto &block : graph_[currentId].domFrontiers) {
                if (!block->phi.count(variable)) {
                    block->phi.insert(variable);
                    if (!defsitesInfo[variable].count(block->id)) {
                        workList.push(block->id);
                    }
                }
            }
        }
    }

    if (IsLogEnabled()) {
        PrintGraph("Inserted Phis");
    }
}

void BytecodeCircuitBuilder::InsertExceptionPhi(std::map<uint16_t, std::set<size_t>> &defsitesInfo)
{
    // handle try catch defsite
    for (auto &bb : graph_) {
        if (bb.isDead) {
            continue;
        }
        if (bb.catchs.size() == 0) {
            continue;
        }
        std::set<size_t> vregs;
        EnumerateBlock(bb, [this, &vregs]
        ([[maybe_unused]]uint8_t * pc, BytecodeInfo &bytecodeInfo) -> bool {
            if (bytecodeInfo.IsBc(EcmaOpcode::RESUMEGENERATOR)) {
                auto numVRegs = method_->GetNumberVRegs();
                for (size_t i = 0; i < numVRegs; i++) {
                    vregs.insert(i);
                }
                return false;
            }
            for (const auto &vreg: bytecodeInfo.vregOut) {
                vregs.insert(vreg);
            }
            return true;
        });

        for (auto &vreg : vregs) {
            defsitesInfo[vreg].insert(bb.catchs.at(0)->id);
            bb.catchs.at(0)->phi.insert(vreg);
        }
    }
}

// Update CFG's predecessor, successor and try catch associations
void BytecodeCircuitBuilder::UpdateCFG()
{
    for (auto &bb: graph_) {
        if (bb.isDead) {
            continue;
        }
        bb.preds.clear();
        bb.trys.clear();
        std::vector<BytecodeRegion *> newSuccs;
        for (const auto &succ: bb.succs) {
            if (std::count(bb.catchs.cbegin(), bb.catchs.cend(), succ)) {
                continue;
            }
            newSuccs.push_back(succ);
        }
        bb.succs = newSuccs;
    }
    for (auto &bb: graph_) {
        if (bb.isDead) {
            continue;
        }
        for (auto &succ: bb.succs) {
            succ->preds.push_back(&bb);
        }
        for (auto &catchBlock: bb.catchs) {
            catchBlock->trys.push_back(&bb);
        }
    }
}

// build circuit
void BytecodeCircuitBuilder::BuildCircuitArgs()
{
    argAcc_.NewCommonArg(CommonArgIdx::GLUE, MachineType::I64, GateType::NJSValue());
    argAcc_.NewCommonArg(CommonArgIdx::LEXENV, MachineType::I64, GateType::TaggedValue());
    argAcc_.NewCommonArg(CommonArgIdx::ACTUAL_ARGC, MachineType::I64, GateType::NJSValue());
    auto funcIdx = static_cast<size_t>(CommonArgIdx::FUNC);
    const size_t actualNumArgs = argAcc_.GetActualNumArgs();
    // new actual argument gates
    for (size_t argIdx = funcIdx; argIdx < actualNumArgs; argIdx++) {
        argAcc_.NewArg(argIdx);
    }
    argAcc_.CollectArgs();
    if (hasTypes_) {
        argAcc_.FillArgsGateType(&typeRecorder_);
        frameStateBuilder_.BuildArgsValues(&argAcc_);
    }
}

bool BytecodeCircuitBuilder::ShouldBeDead(BytecodeRegion &curBlock)
{
    auto isDead = false;
    for (auto bbPred : curBlock.preds) {
        if (!bbPred->isDead) {
            return false;
        }
        isDead = true;
    }
    for (auto bbTry : curBlock.trys) {
        if (!bbTry->isDead) {
            return false;
        }
        isDead = true;
    }
    return isDead;
}

void BytecodeCircuitBuilder::CollectPredsInfo()
{
    for (auto &bb: graph_) {
        if (bb.isDead) {
            continue;
        }
        bb.numOfStatePreds = 0;
    }
    // get number of expanded state predicates of each block
    // one block-level try catch edge may correspond to multiple bytecode-level edges
    for (auto &bb: graph_) {
        if (bb.isDead) {
            continue;
        }
        if (ShouldBeDead(bb)) {
            bb.UpdateTryCatchInfoForDeadBlock();
            bb.isDead = true;
            continue;
        }
        bool noThrow = true;
        EnumerateBlock(bb, [&noThrow, &bb]
        ([[maybe_unused]]uint8_t * pc, BytecodeInfo &bytecodeInfo) -> bool {
            if (bytecodeInfo.IsGeneral()) {
                noThrow = false;
                if (!bb.catchs.empty()) {
                    bb.catchs.at(0)->numOfStatePreds++;
                }
            }
            if (bytecodeInfo.IsCondJump() && bb.succs.size() == 1) {
                ASSERT(bb.succs[0]->id == bb.id + 1);
                bb.succs[0]->numOfStatePreds++;
            }
            return true;
        });
        bb.UpdateRedundantTryCatchInfo(noThrow);
        bb.UpdateTryCatchInfoIfNoThrow(noThrow);
        for (auto &succ: bb.succs) {
            succ->numOfStatePreds++;
        }
    }
    // collect loopback edges
    std::vector<VisitState> visitState(graph_.size(), VisitState::UNVISITED);
    std::function<void(size_t)> dfs = [&](size_t bbId) -> void {
        visitState[bbId] = VisitState::PENDING;
        std::vector<BytecodeRegion *> merge;
        merge.insert(merge.end(), graph_[bbId].succs.begin(), graph_[bbId].succs.end());
        merge.insert(merge.end(), graph_[bbId].catchs.begin(), graph_[bbId].catchs.end());
        auto it = merge.crbegin();
        while (it != merge.crend()) {
            auto succBlock = *it;
            it++;
            if (visitState[succBlock->id] == VisitState::UNVISITED) {
                dfs(succBlock->id);
            } else {
                if (visitState[succBlock->id] == VisitState::PENDING) {
                    graph_[succBlock->id].loopbackBlocks.insert(bbId);
                }
            }
        }
        visitState[bbId] = VisitState::VISITED;
    };
    dfs(graph_[0].id);
    for (auto &bb: graph_) {
        if (bb.isDead) {
            continue;
        }
        bb.phiAcc = (bb.numOfStatePreds > 1) || (!bb.trys.empty());
        bb.numOfLoopBacks = bb.loopbackBlocks.size();
    }
}

void BytecodeCircuitBuilder::NewMerge(GateRef &state, GateRef &depend, size_t numOfIns)
{
    state = circuit_.NewGate(OpCode(OpCode::MERGE), numOfIns,
                             std::vector<GateRef>(numOfIns, Circuit::NullGate()),
                             GateType::Empty());
    depend = circuit_.NewGate(OpCode(OpCode::DEPEND_SELECTOR), numOfIns,
                              std::vector<GateRef>(numOfIns + 1, Circuit::NullGate()),
                              GateType::Empty());
    gateAcc_.NewIn(depend, 0, state);
}

void BytecodeCircuitBuilder::NewLoopBegin(BytecodeRegion &bb)
{
    NewMerge(bb.mergeForwardEdges, bb.depForward, bb.numOfStatePreds - bb.numOfLoopBacks);
    NewMerge(bb.mergeLoopBackEdges, bb.depLoopBack, bb.numOfLoopBacks);
    auto loopBack = circuit_.NewGate(OpCode(OpCode::LOOP_BACK), 0,
                                     {bb.mergeLoopBackEdges}, GateType::Empty());
    bb.stateStart = circuit_.NewGate(OpCode(OpCode::LOOP_BEGIN), 0,
                                     {bb.mergeForwardEdges, loopBack}, GateType::Empty());
    // 2: the number of depend inputs and it is in accord with LOOP_BEGIN
    bb.dependStart = circuit_.NewGate(OpCode(OpCode::DEPEND_SELECTOR), 2,
                                      {bb.stateStart, bb.depForward, bb.depLoopBack},
                                      GateType::Empty());
}

void BytecodeCircuitBuilder::BuildBlockCircuitHead()
{
    for (auto &bb: graph_) {
        if (bb.isDead) {
            continue;
        }
        if (bb.numOfStatePreds == 0) {
            bb.stateStart = Circuit::GetCircuitRoot(OpCode(OpCode::STATE_ENTRY));
            bb.dependStart = Circuit::GetCircuitRoot(OpCode(OpCode::DEPEND_ENTRY));
        } else if (bb.numOfLoopBacks > 0) {
            NewLoopBegin(bb);
        } else {
            NewMerge(bb.stateStart, bb.dependStart, bb.numOfStatePreds);
        }
    }
}

std::vector<GateRef> BytecodeCircuitBuilder::CreateGateInList(const BytecodeInfo &info)
{
    size_t numValueInputs = info.ComputeValueInputCount();
    const size_t length = 2; // 2: state and depend on input
    const size_t numBCOffsetInput = info.ComputeBCOffsetInputCount();
    std::vector<GateRef> inList(length + numValueInputs + numBCOffsetInput, Circuit::NullGate());
    for (size_t i = 0; i < info.inputs.size(); i++) {
        const auto &input = info.inputs[i];
        if (std::holds_alternative<MethodId>(input)) {
            inList[i + length] = circuit_.GetConstantGate(MachineType::I16,
                                                          std::get<MethodId>(input).GetId(),
                                                          GateType::NJSValue());
        } else if (std::holds_alternative<StringId>(input)) {
            tsManager_->AddStringIndex(std::get<StringId>(input).GetId());
            inList[i + length] = circuit_.GetConstantGate(MachineType::I16,
                                                          std::get<StringId>(input).GetId(),
                                                          GateType::NJSValue());
        } else if (std::holds_alternative<Immediate>(input)) {
            inList[i + length] = circuit_.GetConstantGate(MachineType::I64,
                                                          std::get<Immediate>(input).GetValue(),
                                                          GateType::NJSValue());
        } else {
            ASSERT(std::holds_alternative<VirtualRegister>(input));
            continue;
        }
    }
    return inList;
}

void BytecodeCircuitBuilder::SetBlockPred(BytecodeRegion &bbNext, const GateRef &state,
                                          const GateRef &depend, bool isLoopBack)
{
    if (bbNext.numOfLoopBacks == 0) {
        gateAcc_.NewIn(bbNext.stateStart, bbNext.statePredIndex, state);
        gateAcc_.NewIn(bbNext.dependStart, bbNext.statePredIndex + 1, depend);
    } else {
        if (isLoopBack) {
            gateAcc_.NewIn(bbNext.mergeLoopBackEdges, bbNext.loopBackIndex, state);
            gateAcc_.NewIn(bbNext.depLoopBack, bbNext.loopBackIndex + 1, depend);
            bbNext.loopBackIndex++;
            ASSERT(bbNext.loopBackIndex <= bbNext.numOfLoopBacks);
        } else {
            gateAcc_.NewIn(bbNext.mergeForwardEdges, bbNext.forwardIndex, state);
            gateAcc_.NewIn(bbNext.depForward, bbNext.forwardIndex + 1, depend);
            bbNext.forwardIndex++;
            ASSERT(bbNext.forwardIndex <= bbNext.numOfStatePreds - bbNext.numOfLoopBacks);
        }
    }
    bbNext.statePredIndex++;
    ASSERT(bbNext.statePredIndex <= bbNext.numOfStatePreds);
}

GateRef BytecodeCircuitBuilder::NewConst(const BytecodeInfo &info)
{
    auto opcode = info.opcode;
    GateRef gate = 0;
    switch (opcode) {
        case EcmaOpcode::LDNAN:
            gate = circuit_.GetConstantGate(MachineType::I64,
                                            base::NumberHelper::GetNaN(),
                                            GateType::TaggedValue());
            break;
        case EcmaOpcode::LDINFINITY:
            gate = circuit_.GetConstantGate(MachineType::I64,
                                            base::NumberHelper::GetPositiveInfinity(),
                                            GateType::TaggedValue());
            break;
        case EcmaOpcode::LDUNDEFINED:
            gate = circuit_.GetConstantGate(MachineType::I64,
                                            JSTaggedValue::VALUE_UNDEFINED,
                                            GateType::TaggedValue());
            break;
        case EcmaOpcode::LDNULL:
            gate = circuit_.GetConstantGate(MachineType::I64,
                                            JSTaggedValue::VALUE_NULL,
                                            GateType::TaggedValue());
            break;
        case EcmaOpcode::LDTRUE:
            gate = circuit_.GetConstantGate(MachineType::I64,
                                            JSTaggedValue::VALUE_TRUE,
                                            GateType::TaggedValue());
            break;
        case EcmaOpcode::LDFALSE:
            gate = circuit_.GetConstantGate(MachineType::I64,
                                            JSTaggedValue::VALUE_FALSE,
                                            GateType::TaggedValue());
            break;
        case EcmaOpcode::LDHOLE:
            gate = circuit_.GetConstantGate(MachineType::I64,
                                            JSTaggedValue::VALUE_HOLE,
                                            GateType::TaggedValue());
            break;
        case EcmaOpcode::LDAI_IMM32:
            gate = circuit_.GetConstantGate(MachineType::I64,
                                            std::get<Immediate>(info.inputs[0]).ToJSTaggedValueInt(),
                                            GateType::TaggedValue());
            break;
        case EcmaOpcode::FLDAI_IMM64:
            gate = circuit_.GetConstantGate(MachineType::I64,
                                            std::get<Immediate>(info.inputs.at(0)).ToJSTaggedValueDouble(),
                                            GateType::TaggedValue());
            break;
        case EcmaOpcode::LDFUNCTION:
            gate = argAcc_.GetCommonArgGate(CommonArgIdx::FUNC);
            break;
        case EcmaOpcode::LDNEWTARGET:
            gate = argAcc_.GetCommonArgGate(CommonArgIdx::NEW_TARGET);
            break;
        case EcmaOpcode::LDTHIS:
            gate = argAcc_.GetCommonArgGate(CommonArgIdx::THIS);
            break;
        default:
            UNREACHABLE();
    }
    return gate;
}

void BytecodeCircuitBuilder::NewJSGate(BytecodeRegion &bb, const uint8_t *pc, GateRef &state, GateRef &depend)
{
    auto bytecodeInfo = GetBytecodeInfo(pc);
    size_t numValueInputs = bytecodeInfo.ComputeTotalValueCount();
    GateRef gate = 0;
    std::vector<GateRef> inList = CreateGateInList(bytecodeInfo);
    if (bytecodeInfo.IsDef()) {
        gate = circuit_.NewGate(OpCode(OpCode::JS_BYTECODE), MachineType::I64, numValueInputs,
                                inList, GateType::AnyType());
    } else {
        gate = circuit_.NewGate(OpCode(OpCode::JS_BYTECODE), MachineType::NOVALUE, numValueInputs,
                                inList, GateType::Empty());
    }
    // 1: store bcoffset in the end.
    AddBytecodeOffsetInfo(gate, bytecodeInfo, numValueInputs + 1, const_cast<uint8_t *>(pc));
    gateAcc_.NewIn(gate, 0, state);
    gateAcc_.NewIn(gate, 1, depend);
    auto ifSuccess = circuit_.NewGate(OpCode(OpCode::IF_SUCCESS), 0, {gate}, GateType::Empty());
    auto ifException = circuit_.NewGate(OpCode(OpCode::IF_EXCEPTION), 0, {gate}, GateType::Empty());
    if (!bb.catchs.empty()) {
        auto &bbNext = bb.catchs.at(0);
        auto isLoopBack = bbNext->loopbackBlocks.count(bb.id);
        SetBlockPred(*bbNext, ifException, gate, isLoopBack);
        if (bytecodeInfo.opcode == EcmaOpcode::CREATEASYNCGENERATOROBJ_V8) {
            bbNext->expandedPreds.push_back({bb.id, pc + bytecodeInfo.offset, true});
        } else {
            bbNext->expandedPreds.push_back({bb.id, pc, true});
        }
    } else {
        auto constant = circuit_.GetConstantGate(MachineType::I64,
                                                 JSTaggedValue::VALUE_EXCEPTION,
                                                 GateType::TaggedValue());
        circuit_.NewGate(OpCode(OpCode::RETURN), 0,
                         {ifException, gate, constant,
                         Circuit::GetCircuitRoot(OpCode(OpCode::RETURN_LIST))},
                         GateType::Empty());
    }
    jsgateToBytecode_[gate] = {bb.id, pc};
    byteCodeToJSGate_[pc] = gate;
    if (bytecodeInfo.IsGeneratorRelative()) {
        suspendAndResumeGates_.emplace_back(gate);
    }
    if (bytecodeInfo.IsThrow()) {
        auto constant = circuit_.GetConstantGate(MachineType::I64,
                                                 JSTaggedValue::VALUE_EXCEPTION,
                                                 GateType::TaggedValue());
        circuit_.NewGate(OpCode(OpCode::RETURN), 0,
                         {ifSuccess, gate, constant,
                         Circuit::GetCircuitRoot(OpCode(OpCode::RETURN_LIST))},
                         GateType::Empty());
        return;
    }
    state = ifSuccess;
    depend = gate;
    if (pc == bb.end) {
        auto &bbNext = graph_[bb.id + 1];
        auto isLoopBack = bbNext.loopbackBlocks.count(bb.id);
        SetBlockPred(bbNext, state, depend, isLoopBack);
        bbNext.expandedPreds.push_back({bb.id, pc, false});
    }
}

void BytecodeCircuitBuilder::NewJump(BytecodeRegion &bb, const uint8_t *pc, GateRef &state, GateRef &depend)
{
    auto bytecodeInfo = GetBytecodeInfo(pc);
    size_t numValueInputs = bytecodeInfo.ComputeValueInputCount();
    if (bytecodeInfo.IsCondJump()) {
        GateRef gate = 0;
        gate = circuit_.NewGate(OpCode(OpCode::JS_BYTECODE), MachineType::NOVALUE, numValueInputs,
                                std::vector<GateRef>(2 + numValueInputs, // 2: state and depend input
                                                     Circuit::NullGate()),
                                GateType::Empty());
        gateAcc_.NewIn(gate, 0, state);
        gateAcc_.NewIn(gate, 1, depend);
        auto ifTrue = circuit_.NewGate(OpCode(OpCode::IF_TRUE), 0, {gate}, GateType::Empty());
        auto ifFalse = circuit_.NewGate(OpCode(OpCode::IF_FALSE), 0, {gate}, GateType::Empty());
        if (bb.succs.size() == 1) {
            auto &bbNext = bb.succs[0];
            ASSERT(bbNext->id == bb.id + 1);
            auto isLoopBack = bbNext->loopbackBlocks.count(bb.id);
            SetBlockPred(*bbNext, ifFalse, gate, isLoopBack);
            SetBlockPred(*bbNext, ifTrue, gate, isLoopBack);
            bbNext->expandedPreds.push_back({bb.id, pc, false});
        } else {
            ASSERT(bb.succs.size() == 2); // 2 : 2 num of successors
            [[maybe_unused]] uint32_t bitSet = 0;
            for (auto &bbNext: bb.succs) {
                if (bbNext->id == bb.id + 1) {
                    auto isLoopBack = bbNext->loopbackBlocks.count(bb.id);
                    SetBlockPred(*bbNext, ifFalse, gate, isLoopBack);
                    bbNext->expandedPreds.push_back({bb.id, pc, false});
                    bitSet |= 1;
                } else {
                    auto isLoopBack = bbNext->loopbackBlocks.count(bb.id);
                    SetBlockPred(*bbNext, ifTrue, gate, isLoopBack);
                    bbNext->expandedPreds.push_back({bb.id, pc, false});
                    bitSet |= 2; // 2:verify
                }
            }
            ASSERT(bitSet == 3); // 3:Verify the number of successor blocks
        }
        jsgateToBytecode_[gate] = {bb.id, pc};
        byteCodeToJSGate_[pc] = gate;
    } else {
        ASSERT(bb.succs.size() == 1);
        auto &bbNext = bb.succs.at(0);
        auto isLoopBack = bbNext->loopbackBlocks.count(bb.id);
        SetBlockPred(*bbNext, state, depend, isLoopBack);
        bbNext->expandedPreds.push_back({bb.id, pc, false});
    }
}

void BytecodeCircuitBuilder::NewReturn(BytecodeRegion &bb, const uint8_t *pc, GateRef &state, GateRef &depend)
{
    ASSERT(bb.succs.empty());
    auto bytecodeInfo = GetBytecodeInfo(pc);
    if (bytecodeInfo.opcode == EcmaOpcode::RETURN) {
        // handle return.dyn bytecode
        auto gate = circuit_.NewGate(OpCode(OpCode::RETURN), 0,
                                     { state, depend, Circuit::NullGate(),
                                     Circuit::GetCircuitRoot(OpCode(OpCode::RETURN_LIST)) },
                                     GateType::Empty());
        jsgateToBytecode_[gate] = {bb.id, pc};
        byteCodeToJSGate_[pc] = gate;
    } else if (bytecodeInfo.opcode == EcmaOpcode::RETURNUNDEFINED) {
        // handle returnundefined bytecode
        auto constant = circuit_.GetConstantGate(MachineType::I64,
                                                 JSTaggedValue::VALUE_UNDEFINED,
                                                 GateType::TaggedValue());
        auto gate = circuit_.NewGate(OpCode(OpCode::RETURN), 0,
                                     { state, depend, constant,
                                     Circuit::GetCircuitRoot(OpCode(OpCode::RETURN_LIST)) },
                                     GateType::Empty());
        jsgateToBytecode_[gate] = {bb.id, pc};
        byteCodeToJSGate_[pc] = gate;
    }
}

void BytecodeCircuitBuilder::NewByteCode(BytecodeRegion &bb, const uint8_t *pc, GateRef &state, GateRef &depend)
{
    ASSERT(pc != nullptr);
    auto bytecodeInfo = GetBytecodeInfo(pc);
    if (bytecodeInfo.IsSetConstant()) {
        // handle bytecode command to get constants
        GateRef gate = NewConst(bytecodeInfo);
        jsgateToBytecode_[gate] = {bb.id, pc};
        byteCodeToJSGate_[pc] = gate;
        if (pc == bb.end) {
            auto &bbNext = graph_[bb.id + 1];
            auto isLoopBack = bbNext.loopbackBlocks.count(bb.id);
            SetBlockPred(bbNext, state, depend, isLoopBack);
            bbNext.expandedPreds.push_back({bb.id, pc, false});
        }
    } else if (bytecodeInfo.IsGeneral()) {
        // handle general ecma.* bytecodes
        NewJSGate(bb, pc, state, depend);
    } else if (bytecodeInfo.IsJump()) {
        // handle conditional jump and unconditional jump bytecodes
        NewJump(bb, pc, state, depend);
    } else if (bytecodeInfo.IsReturn()) {
        // handle return.dyn and returnundefined bytecodes
        NewReturn(bb, pc, state, depend);
    } else if (bytecodeInfo.IsMov()) {
        // handle mov.dyn lda.dyn sta.dyn bytecodes
        if (pc == bb.end) {
            auto &bbNext = graph_[bb.id + 1];
            auto isLoopBack = bbNext.loopbackBlocks.count(bb.id);
            SetBlockPred(bbNext, state, depend, isLoopBack);
            bbNext.expandedPreds.push_back({bb.id, pc, false});
        }
    } else if (bytecodeInfo.IsDiscarded()) {
        return;
    } else {
        UNREACHABLE();
    }
}

void BytecodeCircuitBuilder::BuildSubCircuit()
{
    for (auto &bb: graph_) {
        if (bb.isDead) {
            continue;
        }
        auto stateCur = bb.stateStart;
        auto dependCur = bb.dependStart;
        ASSERT(stateCur != Circuit::NullGate());
        ASSERT(dependCur != Circuit::NullGate());
        if (!bb.trys.empty()) {
            dependCur = circuit_.NewGate(OpCode(OpCode::GET_EXCEPTION), 0, {dependCur}, GateType::Empty());
        }
        EnumerateBlock(bb, [this, &stateCur, &dependCur, &bb]
        (uint8_t * pc, BytecodeInfo &bytecodeInfo) -> bool {
            NewByteCode(bb, pc, stateCur, dependCur);
            if (bytecodeInfo.IsJump() || bytecodeInfo.IsThrow()) {
                return false;
            }
            return true;
        });
    }
}

void BytecodeCircuitBuilder::NewPhi(BytecodeRegion &bb, uint16_t reg, bool acc, GateRef &currentPhi)
{
    if (bb.numOfLoopBacks == 0) {
        currentPhi =
            circuit_.NewGate(OpCode(OpCode::VALUE_SELECTOR), MachineType::I64, bb.numOfStatePreds,
                             std::vector<GateRef>(1 + bb.numOfStatePreds, Circuit::NullGate()), GateType::AnyType());
        gateAcc_.NewIn(currentPhi, 0, bb.stateStart);
        for (size_t i = 0; i < bb.numOfStatePreds; ++i) {
            auto &[predId, predPc, isException] = bb.expandedPreds.at(i);
            gateAcc_.NewIn(currentPhi, i + 1, RenameVariable(predId, predPc, reg, acc));
        }
    } else {
        // 2: the number of value inputs and it is in accord with LOOP_BEGIN
        currentPhi = circuit_.NewGate(OpCode(OpCode::VALUE_SELECTOR), MachineType::I64, 2,
                                      {bb.stateStart, Circuit::NullGate(), Circuit::NullGate()}, GateType::AnyType());
        auto loopBackValue =
            circuit_.NewGate(OpCode(OpCode::VALUE_SELECTOR), MachineType::I64, bb.numOfLoopBacks,
                             std::vector<GateRef>(1 + bb.numOfLoopBacks, Circuit::NullGate()), GateType::AnyType());
        gateAcc_.NewIn(loopBackValue, 0, bb.mergeLoopBackEdges);
        size_t loopBackIndex = 1;  // 1: start index of value inputs
        for (size_t i = 0; i < bb.numOfStatePreds; ++i) {
            auto &[predId, predPc, isException] = bb.expandedPreds.at(i);
            if (bb.loopbackBlocks.count(predId)) {
                gateAcc_.NewIn(loopBackValue, loopBackIndex++, RenameVariable(predId, predPc, reg, acc));
            }
        }
        auto forwardValue = circuit_.NewGate(
            OpCode(OpCode::VALUE_SELECTOR), MachineType::I64, bb.numOfStatePreds - bb.numOfLoopBacks,
            std::vector<GateRef>(1 + bb.numOfStatePreds - bb.numOfLoopBacks, Circuit::NullGate()), GateType::AnyType());
        gateAcc_.NewIn(forwardValue, 0, bb.mergeForwardEdges);
        size_t forwardIndex = 1;  // 1: start index of value inputs
        for (size_t i = 0; i < bb.numOfStatePreds; ++i) {
            auto &[predId, predPc, isException] = bb.expandedPreds.at(i);
            if (!bb.loopbackBlocks.count(predId)) {
                gateAcc_.NewIn(forwardValue, forwardIndex++, RenameVariable(predId, predPc, reg, acc));
            }
        }
        gateAcc_.NewIn(currentPhi, 1, forwardValue);   // 1: index of forward value input
        gateAcc_.NewIn(currentPhi, 2, loopBackValue);  // 2: index of loop-back value input
    }
}

// recursive variables renaming algorithm
GateRef BytecodeCircuitBuilder::RenameVariable(const size_t bbId, const uint8_t *end,
                                               const uint16_t reg, const bool acc)
{
    ASSERT(end != nullptr);
    auto tmpReg = reg;
    // find def-site in bytecodes of basic block
    auto ans = Circuit::NullGate();
    auto &bb = graph_.at(bbId);
    std::vector<uint8_t *> instList;
    {
        auto pcIter = bb.start;
        while (pcIter <= end) {
            instList.push_back(pcIter);
            auto curInfo = GetBytecodeInfo(pcIter);
            pcIter += curInfo.offset;
        }
    }
    GateType type = GateType::AnyType();
    auto tmpAcc = acc;
    for (auto pcIter = instList.rbegin(); pcIter != instList.rend(); pcIter++) { // upper bound
        auto curInfo = GetBytecodeInfo(*pcIter);
        // original bc use acc as input && current bc use acc as output
        bool isTransByAcc = tmpAcc && curInfo.accOut;
        // 0 : the index in vreg-out list
        bool isTransByVreg = (!tmpAcc && curInfo.IsOut(tmpReg, 0));
        if (isTransByAcc || isTransByVreg) {
            if (curInfo.IsMov()) {
                tmpAcc = curInfo.accIn;
                if (!curInfo.inputs.empty()) {
                    ASSERT(!tmpAcc);
                    ASSERT(curInfo.inputs.size() == 1);
                    tmpReg = std::get<VirtualRegister>(curInfo.inputs.at(0)).GetId();
                }
                if (hasTypes_) {
                    type = typeRecorder_.UpdateType(pcToBCOffset_.at(*pcIter) - 1, type);
                }
            } else {
                ans = byteCodeToJSGate_.at(*pcIter);
                if (hasTypes_ && !type.IsAnyType()) {
                    gateAcc_.SetGateType(ans, type);
                }
                break;
            }
        }
        if (curInfo.opcode != EcmaOpcode::RESUMEGENERATOR) {
            continue;
        }
        // New RESTORE_REGISTER HIR, used to restore the register content when processing resume instruction.
        // New SAVE_REGISTER HIR, used to save register content when processing suspend instruction.
        auto resumeGate = byteCodeToJSGate_.at(*pcIter);
        ans = GetExistingRestore(resumeGate, tmpReg);
        if (ans != Circuit::NullGate()) {
            break;
        }
        GateRef resumeDependGate = gateAcc_.GetDep(resumeGate);
        ans = circuit_.NewGate(OpCode(OpCode::RESTORE_REGISTER), MachineType::I64, tmpReg,
                               {resumeDependGate}, GateType::AnyType());
        SetExistingRestore(resumeGate, tmpReg, ans);
        gateAcc_.SetDep(resumeGate, ans);
        auto saveRegGate = RenameVariable(bbId, *pcIter - 1, tmpReg, tmpAcc);
        auto nextPcIter = pcIter;
        nextPcIter++;
        nextPcIter++;
        ASSERT(GetBytecodeInfo(*nextPcIter).opcode == EcmaOpcode::SUSPENDGENERATOR_V8);
        GateRef suspendGate = byteCodeToJSGate_.at(*nextPcIter);
        auto dependGate = gateAcc_.GetDep(suspendGate);
        auto newDependGate = circuit_.NewGate(OpCode(OpCode::SAVE_REGISTER), tmpReg, {dependGate, saveRegGate},
                                              GateType::Empty());
        gateAcc_.SetDep(suspendGate, newDependGate);
        break;
    }
    // find GET_EXCEPTION gate if this is a catch block
    if (ans == Circuit::NullGate() && tmpAcc) {
        if (!bb.trys.empty()) {
            std::vector<GateRef> outList;
            gateAcc_.GetOutVector(bb.dependStart, outList);
            ASSERT(outList.size() == 1);
            const auto &getExceptionGate = outList.at(0);
            ASSERT(gateAcc_.GetOpCode(getExceptionGate) == OpCode::GET_EXCEPTION);
            ans = getExceptionGate;
        }
    }
    // find def-site in value selectors of vregs
    if (ans == Circuit::NullGate() && !tmpAcc && bb.phi.count(tmpReg)) {
        if (!bb.vregToValSelectorGate.count(tmpReg)) {
            NewPhi(bb, tmpReg, tmpAcc, bb.vregToValSelectorGate[tmpReg]);
        }
        ans = bb.vregToValSelectorGate.at(tmpReg);
    }
    // find def-site in value selectors of acc
    if (ans == Circuit::NullGate() && tmpAcc && bb.phiAcc) {
        if (bb.valueSelectorAccGate == Circuit::NullGate()) {
            NewPhi(bb, tmpReg, tmpAcc, bb.valueSelectorAccGate);
        }
        ans = bb.valueSelectorAccGate;
    }
    if (ans == Circuit::NullGate() && IsEntryBlock(bbId)) { // entry block
        // find def-site in function args
        ASSERT(!tmpAcc);
        ans = argAcc_.GetArgGate(tmpReg);
        return ans;
    }
    if (ans == Circuit::NullGate()) {
        // recursively find def-site in dominator block
        return RenameVariable(bb.iDominator->id, bb.iDominator->end, tmpReg, tmpAcc);
    } else {
        // def-site already found
        return ans;
    }
}

void BytecodeCircuitBuilder::BuildFrameState()
{
    const uint8_t *predPc = nullptr;
    for (auto &bb: graph_) {
        if (bb.isDead) {
            continue;
        }
        if (bb.numOfStatePreds != 0) {
            predPc = std::get<const uint8_t *>(bb.expandedPreds.at(0));
        }
        frameStateBuilder_.AdvenceToSuccessor(predPc, bb.end);
        if (bb.valueSelectorAccGate != Circuit::NullGate()) {
            frameStateBuilder_.UpdateAccumulator(bb.valueSelectorAccGate);
        }
        for (auto &it : bb.vregToValSelectorGate) {
            auto reg = it.first;
            auto gate = it.second;
            frameStateBuilder_.UpdateVirtualRegister(reg, gate);
        }
        EnumerateBlock(bb, [this](uint8_t * pc, BytecodeInfo &bytecodeInfo) -> bool {
            GateRef gate = Circuit::NullGate();
            if (bytecodeInfo.IsMov()) {
                if (bytecodeInfo.accIn) {
                    gate = frameStateBuilder_.ValuesAtAccumulator();
                } else if (bytecodeInfo.inputs.size() != 0) {
                    auto index = std::get<VirtualRegister>(bytecodeInfo.inputs.at(0)).GetId();
                    gate = frameStateBuilder_.ValuesAt(index);
                }
            } else if (bytecodeInfo.IsGeneral()) {
                gate = byteCodeToJSGate_.at(pc);
                if (bytecodeInfo.deopt) {
                    frameStateBuilder_.BindGuard(gate, bytecodeInfo.pcOffset);
                }
            } else if (bytecodeInfo.IsSetConstant()) {
                gate = byteCodeToJSGate_.at(pc);
            }
            if (bytecodeInfo.accOut) {
                frameStateBuilder_.UpdateAccumulator(gate);
            }
            for (const auto &out: bytecodeInfo.vregOut) {
                frameStateBuilder_.UpdateVirtualRegister(out, gate);
            }
            return true;
        });
    }
}

void BytecodeCircuitBuilder::BuildCircuit()
{
    // create arg gates array
    BuildCircuitArgs();
    CollectPredsInfo();
    BuildBlockCircuitHead();
    // build states sub-circuit of each block
    BuildSubCircuit();
    // verification of soundness of CFG
    for (auto &bb: graph_) {
        if (bb.isDead) {
            continue;
        }
        ASSERT(bb.statePredIndex == bb.numOfStatePreds);
        ASSERT(bb.loopBackIndex == bb.numOfLoopBacks);
        if (bb.numOfLoopBacks) {
            ASSERT(bb.forwardIndex == bb.numOfStatePreds - bb.numOfLoopBacks);
        }
    }
    // resolve def-site of virtual regs and set all value inputs
    std::vector<GateRef> gates;
    circuit_.GetAllGates(gates);
    for (auto gate: gates) {
        auto valueCount = gateAcc_.GetInValueCount(gate);
        auto it = jsgateToBytecode_.find(gate);
        if (it == jsgateToBytecode_.cend()) {
            continue;
        }
        if (gateAcc_.GetOpCode(gate) == OpCode::CONSTANT) {
            continue;
        }
        const auto &[id, pc] = it->second;
        if (hasTypes_) {
            auto type = typeRecorder_.GetType(pcToBCOffset_.at(pc) - 1);
            if (!type.IsAnyType()) {
                gateAcc_.SetGateType(gate, type);
            }
        }
        auto bytecodeInfo = GetBytecodeInfo(pc);
        [[maybe_unused]] size_t numValueInputs = bytecodeInfo.ComputeTotalValueCount();
        [[maybe_unused]] size_t numValueOutputs = bytecodeInfo.ComputeOutCount() + bytecodeInfo.vregOut.size();
        ASSERT(numValueInputs == valueCount);
        ASSERT(numValueOutputs <= 1);
        auto stateCount = gateAcc_.GetStateCount(gate);
        auto dependCount = gateAcc_.GetDependCount(gate);
        for (size_t valueIdx = 0; valueIdx < valueCount; valueIdx++) {
            auto inIdx = valueIdx + stateCount + dependCount;
            if (!gateAcc_.IsInGateNull(gate, inIdx)) {
                continue;
            }
            if (valueIdx < bytecodeInfo.inputs.size()) {
                auto vregId = std::get<VirtualRegister>(bytecodeInfo.inputs.at(valueIdx)).GetId();
                GateRef defVreg = RenameVariable(id, pc - 1, vregId, false);
                gateAcc_.NewIn(gate, inIdx, defVreg);
            } else {
                GateRef defAcc = RenameVariable(id, pc - 1, 0, true);
                gateAcc_.NewIn(gate, inIdx, defAcc);
            }
        }
    }
    if (hasTypes_) {
        BuildFrameState();
    }

    if (IsLogEnabled()) {
        PrintGraph("Bytecode2Gate");
        LOG_COMPILER(INFO) << "\033[34m" << "============= "
                           << "After bytecode2circuit lowering ["
                           << methodName_ << "]"
                           << " =============" << "\033[0m";
        circuit_.PrintAllGates(*this);
        LOG_COMPILER(INFO) << "\033[34m" << "=========================== End ===========================" << "\033[0m";
    }
}

void BytecodeCircuitBuilder::AddBytecodeOffsetInfo(GateRef &gate, const BytecodeInfo &info, size_t bcOffsetIndex,
                                                   uint8_t *pc)
{
    if (info.IsCall()) {
        auto bcOffset = circuit_.GetConstantGate(MachineType::I64,
                                                 pcToBCOffset_.at(pc),
                                                 GateType::NJSValue());
        gateAcc_.NewIn(gate, bcOffsetIndex, bcOffset);
    }
}

GateRef BytecodeCircuitBuilder::GetExistingRestore(GateRef resumeGate, uint16_t tmpReg) const
{
    auto pr = std::make_pair(resumeGate, tmpReg);
    if (resumeRegToRestore_.count(pr)) {
        return resumeRegToRestore_.at(pr);
    }
    return Circuit::NullGate();
}

void BytecodeCircuitBuilder::SetExistingRestore(GateRef resumeGate, uint16_t tmpReg, GateRef restoreGate)
{
    auto pr = std::make_pair(resumeGate, tmpReg);
    resumeRegToRestore_[pr] = restoreGate;
}

void BytecodeCircuitBuilder::PrintGraph(const char* title)
{
    std::map<const uint8_t *, GateRef> bcToGate;
    for (const auto &[key, value]: jsgateToBytecode_) {
        bcToGate[value.second] = key;
    }

    LOG_COMPILER(INFO) << "======================== " << title << " ========================";
    for (size_t i = 0; i < graph_.size(); i++) {
        BytecodeRegion& bb = graph_[i];
        if (bb.isDead) {
            LOG_COMPILER(INFO) << "B" << bb.id << ":                               ;preds= invalid BB";
            LOG_COMPILER(INFO) << "\tBytecodePC: [" << reinterpret_cast<void*>(bb.start) << ", "
                               << reinterpret_cast<void*>(bb.end) << ")";
            continue;
        }
        std::string log("B" + std::to_string(bb.id) + ":                               ;preds= ");
        for (size_t k = 0; k < bb.preds.size(); ++k) {
            log += std::to_string(bb.preds[k]->id) + ", ";
        }
        LOG_COMPILER(INFO) << log;
        LOG_COMPILER(INFO) << "\tBytecodePC: [" << reinterpret_cast<void*>(bb.start) << ", "
                           << reinterpret_cast<void*>(bb.end) << ")";

        std::string log1("\tSucces: ");
        for (size_t j = 0; j < bb.succs.size(); j++) {
            log1 += std::to_string(bb.succs[j]->id) + ", ";
        }
        LOG_COMPILER(INFO) << log1;

        for (size_t j = 0; j < bb.catchs.size(); j++) {
            LOG_COMPILER(INFO) << "\tcatch [: " << reinterpret_cast<void*>(bb.catchs[j]->start) << ", "
                               << reinterpret_cast<void*>(bb.catchs[j]->end) << ")";
        }

        std::string log2("\tTrys: ");
        for (auto tryBlock: bb.trys) {
            log2 += std::to_string(tryBlock->id) + " , ";
        }
        LOG_COMPILER(INFO) << log2;

        std::string log3 = "\tDom: ";
        for (size_t j = 0; j < bb.immDomBlocks.size(); j++) {
            log3 += "B" + std::to_string(bb.immDomBlocks[j]->id) + std::string(", ");
        }
        LOG_COMPILER(INFO) << log3;

        if (bb.iDominator) {
            LOG_COMPILER(INFO) << "\tIDom B" << bb.iDominator->id;
        }

        std::string log4("\tDom Frontiers: ");
        for (const auto &frontier: bb.domFrontiers) {
            log4 += std::to_string(frontier->id) + " , ";
        }
        LOG_COMPILER(INFO) << log4;

        std::string log5("\tPhi: ");
        for (auto variable: bb.phi) {
            log5 += std::to_string(variable) + " , ";
        }
        LOG_COMPILER(INFO) << log5;

        PrintBytecodeInfo(bb, bcToGate);
        LOG_COMPILER(INFO) << "";
    }
}

void BytecodeCircuitBuilder::PrintBytecodeInfo(BytecodeRegion& bb, const std::map<const uint8_t *, GateRef>& bcToGate)
{
    if (bb.isDead) {
        return;
    }
    LOG_COMPILER(INFO) << "\tBytecode[] = ";
    EnumerateBlock(bb, [=](uint8_t * pc, BytecodeInfo &bytecodeInfo) -> bool {
        std::string log;
        log += std::string("\t\t< ") + GetEcmaOpcodeStr(static_cast<EcmaOpcode>(*pc)) + ", " + "In=[";
        if (bytecodeInfo.accIn) {
            log += "acc,";
        }
        for (const auto &in: bytecodeInfo.inputs) {
            if (std::holds_alternative<VirtualRegister>(in)) {
                log += std::to_string(std::get<VirtualRegister>(in).GetId()) + ",";
            }
        }
        log += "], Out=[";
        if (bytecodeInfo.accOut) {
            log += "acc,";
        }
        for (const auto &out: bytecodeInfo.vregOut) {
            log +=  std::to_string(out) + ",";
        }
        log += "] >";
        LOG_COMPILER(INFO) << log;

        auto r = bcToGate.find(pc);
        if (r != bcToGate.end()) {
            this->gateAcc_.ShortPrint(r->second);
        }
        return true;
    });
}
}  // namespace panda::ecmascript::kungfu
