/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "ecmascript/compiler/type_recorder.h"

#include "ecmascript/compiler/ts_hclass_generator.h"
#include "ecmascript/jspandafile/type_literal_extractor.h"
#include "ecmascript/pgo_profiler/pgo_profiler_decoder.h"
#include "ecmascript/pgo_profiler/pgo_profiler_layout.h"
#include "ecmascript/pgo_profiler/types/pgo_profiler_type.h"
#include "ecmascript/ts_types/ts_type_parser.h"

namespace panda::ecmascript::kungfu {
using PGOType = pgo::PGOType;
using PGOObjectInfo = pgo::PGOObjectInfo;
TypeRecorder::TypeRecorder(const JSPandaFile *jsPandaFile, const MethodLiteral *methodLiteral,
                           TSManager *tsManager, const CString &recordName, PGOProfilerDecoder *decoder,
                           const MethodPcInfo &methodPCInfo, const Bytecodes *bytecodes, bool enableOptTrackField)
    : argTypes_(methodLiteral->GetNumArgsWithCallField() + static_cast<size_t>(TypedArgIdx::NUM_OF_TYPED_ARGS),
    GateType::AnyType()), decoder_(decoder), enableOptTrackField_(enableOptTrackField),
    pcOffsets_(methodPCInfo.pcOffsets), bytecodes_(bytecodes)
{
    TSHClassGenerator generator(tsManager);
    if (jsPandaFile->HasTSTypes(recordName)) {
        LoadTypes(jsPandaFile, methodLiteral, tsManager, recordName);
    }
    LoadTypesFromPGO(jsPandaFile, methodLiteral, recordName);
    CreateTypesForPGO(jsPandaFile, methodLiteral, tsManager, recordName);
    generator.GenerateTSHClasses();
}

void TypeRecorder::LoadTypes(const JSPandaFile *jsPandaFile, const MethodLiteral *methodLiteral,
                             TSManager *tsManager, const CString &recordName)
{
    TSTypeParser typeParser(tsManager);
    panda_file::File::EntityId fieldId = methodLiteral->GetMethodId();
    uint32_t methodOffset = fieldId.GetOffset();
    TypeAnnotationExtractor annoExtractor(jsPandaFile, methodOffset);
    GlobalTSTypeRef funcGT = typeParser.CreateGT(jsPandaFile, recordName, annoExtractor.GetMethodTypeOffset());
    GlobalTSTypeRef thisGT;
    annoExtractor.EnumerateInstsAndTypes([this, &typeParser, &jsPandaFile, &recordName,
        &thisGT, tsManager, methodOffset](const int32_t bcIdx, const uint32_t typeId) {
        GlobalTSTypeRef gt = typeParser.CreateGT(jsPandaFile, recordName, typeId);
        if (TypeNeedFilter(gt)) {
            return;
        }
        TypeLocation loc(jsPandaFile, methodOffset, bcIdx);
        CollectLiteralGT(tsManager, loc, gt);

        // The type of a function is recorded as (-1, funcTypeId). If the function is a member of a class,
        // the type of the class or its instance is is recorded as (-2, classTypeId). If it is a static
        // member, the type id refers to the type of the class; otherwise, it links to the type of the
        // instances of the class.
        if (bcIdx == METHOD_ANNOTATION_THIS_TYPE_OFFSET) {
            thisGT = gt;
            return;
        }
        auto type = GateType(gt);
        bcOffsetGtMap_.emplace(bcIdx, type);
    });
    const auto &methodList = typeParser.GetMethodList();
    auto methodIter = methodList.find(methodOffset);
    if (methodIter != methodList.end()) {
        const auto &methodInfo = methodIter->second;
        const auto &bcTypes = methodInfo.GetBCAndTypes();
        for (const auto &pair : bcTypes) {
            GlobalTSTypeRef gt = typeParser.CreateGT(jsPandaFile, recordName, pair.second);
            // if the function type has already recorded in the next pc, we should skip it.
            if (CheckTypeMarkForDefineFunc(pair.first)) {
                continue;
            }
            bcOffsetGtMap_.emplace(pair.first, GateType(gt));
        }
    }
    LoadArgTypes(tsManager, funcGT, thisGT);
}

void TypeRecorder::CollectLiteralGT(TSManager *tsManager, TypeLocation &loc, GlobalTSTypeRef gt)
{
    int32_t bcIdx = loc.GetBcIdx();
    if (bcIdx < 0 || bcIdx >= static_cast<int32_t>(pcOffsets_.size())) {
        return;
    }

    while (bytecodes_->GetOpcode(pcOffsets_[bcIdx]) == EcmaOpcode::STA_V8) {
        // bcIndex of literal marked in es2abc maybe in the next bc whose opcode should be sta.
        bcIdx--;
        loc.SetBcIdx(bcIdx);
    }

    EcmaOpcode ecmaOpcode =  bytecodes_->GetOpcode(pcOffsets_[bcIdx]);

    switch (ecmaOpcode) {
        case BytecodeInstruction::Opcode::DEFINECLASSWITHBUFFER_IMM16_ID16_ID16_IMM16_V8:
        case BytecodeInstruction::Opcode::DEFINECLASSWITHBUFFER_IMM8_ID16_ID16_IMM16_V8: {
            if (tsManager->IsUserDefinedClassTypeKind(gt)) {
                tsManager->InsertLiteralGTMap(loc, gt);
            }
            return;
        }
        case BytecodeInstruction::Opcode::CREATEOBJECTWITHBUFFER_IMM8_ID16:
        case BytecodeInstruction::Opcode::CREATEOBJECTWITHBUFFER_IMM16_ID16: {
            if (tsManager->IsObjectTypeKind(gt)) {
                tsManager->InsertLiteralGTMap(loc, gt);
            }
            return;
        }
        default:
            return;
    }
}

bool TypeRecorder::CheckTypeMarkForDefineFunc(uint32_t checkBc) const
{
    // bcOffset of definefunc marked in es2abc maybe in the next bc whose opcode should be sta.
    uint32_t staBc = checkBc + 1;
    return bcOffsetGtMap_.find(staBc) != bcOffsetGtMap_.end() &&
        staBc < pcOffsets_.size() && bytecodes_->GetOpcode(pcOffsets_[staBc]) == EcmaOpcode::STA_V8;
}

void TypeRecorder::LoadTypesFromPGO(const JSPandaFile *jsPandaFile, const MethodLiteral *methodLiteral,
                                    const CString &recordName)
{
    auto callback = [this] (uint32_t offset, PGOType *type) {
        if (type->IsScalarOpType()) {
            bcOffsetPGOOpTypeMap_[offset] = *reinterpret_cast<PGOSampleType *>(type);
        } else if (type->IsRwOpType()) {
            bcOffsetPGORwTypeMap_[offset] = *reinterpret_cast<PGORWOpType *>(type);
        } else {
            UNREACHABLE();
        }
    };
    if (decoder_ != nullptr) {
        decoder_->GetTypeInfo(jsPandaFile, recordName, methodLiteral, callback);
    }
}

void TypeRecorder::CreateTypesForPGO(const JSPandaFile *jsPandaFile, const MethodLiteral *methodLiteral,
                                     TSManager *tsManager, const CString &recordName)
{
    TSTypeParser typeParser(tsManager);
    uint32_t methodOffset = methodLiteral->GetMethodId().GetOffset();
    PGOBCInfo *bcInfo = tsManager->GetBytecodeInfoCollector()->GetPGOBCInfo();
    bcInfo->IterateInfoAndType(methodOffset, [this, &typeParser, methodOffset, &recordName, &jsPandaFile, tsManager]
        (const PGOBCInfo::Type type, const uint32_t bcIdx, const uint32_t bcOffset, const uint32_t cpIdx) {
        auto it = bcOffsetPGOOpTypeMap_.find(bcOffset);
        if (it == bcOffsetPGOOpTypeMap_.end()) {
            return;
        }

        EcmaOpcode ecmaOpcode = bytecodes_->GetOpcode(pcOffsets_[bcIdx]);
        if (jsPandaFile->HasTSTypes(recordName) && Bytecodes::IsCallOp(ecmaOpcode)) {
            auto profile = it->second.GetProfileType();
            if (!profile.IsMethodId()) {
                return;
            }
            uint32_t callTargetMethodOffset = profile.GetId();
            if (callTargetMethodOffset == 0) {
                return;
            }
            // Recompiling the application ABC changes the content, and there may be a calltargetMethodOffset
            // that does not exist in ABC. Type resolution should be skipped when it does not exist,
            // or parsing pandafile will fail
            if (jsPandaFile->GetMethodLiteralByIndex(callTargetMethodOffset) == nullptr) {
                return;
            }
            TypeAnnotationExtractor annoExtractor(jsPandaFile, callTargetMethodOffset);
            GlobalTSTypeRef funcGT =
                typeParser.CreateGT(jsPandaFile, recordName, annoExtractor.GetMethodTypeOffset());
            if (funcGT.IsDefault()) {
                return;
            }
            // Target method was not compiled by AOT.
            if (!decoder_->Match(jsPandaFile, recordName, pgo::PGOMethodId(callTargetMethodOffset))) {
                tsManager->SetHotnessFunc(funcGT, false);
            }
            GateType callTargetType = GateType(funcGT);
            bcOffsetCallTargetGtMap_.emplace(bcIdx, callTargetType);
            return;
        }

        TypeLocation loc(jsPandaFile, methodOffset, bcIdx);
        if (!tsManager->GetLiteralGT(loc).IsDefault()) {
            return;
        }

        GlobalTSTypeRef gt = typeParser.CreatePGOGT(TSTypeParser::PGOInfo {
            jsPandaFile, recordName, methodOffset, cpIdx, it->second, type, decoder_, enableOptTrackField_ });
        if (TypeNeedFilter(gt)) {
            return;
        }
        CollectLiteralGT(tsManager, loc, gt);
    });
}

void TypeRecorder::BindPgoTypeToGateType(const JSPandaFile *jsPandaFile, TSManager *tsManager,
                                         const MethodLiteral *methodLiteral) const
{
    uint32_t methodOffset = methodLiteral->GetMethodId().GetOffset();
    PGOBCInfo *bcInfo = tsManager->GetBytecodeInfoCollector()->GetPGOBCInfo();
    bcInfo->IterateInfoAndType(methodOffset, [this, methodOffset, &jsPandaFile, tsManager]
        (const PGOBCInfo::Type, const uint32_t bcIdx, const uint32_t bcOffset, const uint32_t) {
        auto it = bcOffsetPGOOpTypeMap_.find(bcOffset);
        if (it == bcOffsetPGOOpTypeMap_.end()) {
            return;
        }
        TypeLocation loc(jsPandaFile, methodOffset, bcIdx);
        if (!tsManager->GetLiteralGT(loc).IsDefault()) {
            GlobalTSTypeRef gt = tsManager->GetLiteralGT(loc);
            PGOHClassLayoutDesc *desc;
            if (decoder_->GetHClassLayoutDesc(it->second, &desc)) {
                GateType gateType(gt);
                tsManager->InsertPtToGtMap(desc->GetProfileType(), gateType);
                TSHClassGenerator generator(tsManager);
                generator.UpdateTSHClassFromPGO(gateType, *desc, enableOptTrackField_);
            }
        }
    });
}

void TypeRecorder::LoadArgTypes(const TSManager *tsManager, GlobalTSTypeRef funcGT, GlobalTSTypeRef thisGT)
{
    argTypes_[static_cast<size_t>(TypedArgIdx::FUNC)] = TryGetFuncType(funcGT);
    argTypes_[static_cast<size_t>(TypedArgIdx::NEW_TARGET)] = TryGetNewTargetType(tsManager, thisGT);
    argTypes_[static_cast<size_t>(TypedArgIdx::THIS_OBJECT)] = TryGetThisType(tsManager, funcGT, thisGT);

    if (funcGT.IsDefault()) {
        return;
    }
    size_t extraParasNum = static_cast<size_t>(TypedArgIdx::NUM_OF_TYPED_ARGS);
    uint32_t numExplicitArgs = tsManager->GetFunctionTypeLength(funcGT);
    for (uint32_t explicitArgId = 0; explicitArgId < numExplicitArgs; explicitArgId++) {
        argTypes_[extraParasNum++] = GateType(tsManager->GetFuncParameterTypeGT(funcGT, explicitArgId));
    }
}

GateType TypeRecorder::TryGetThisType(const TSManager *tsManager, GlobalTSTypeRef funcGT, GlobalTSTypeRef thisGT) const
{
    // The parameter 'this' may be declared explicitly, e.g. foo(this: Person, num: number). In this case, the type
    // of 'this' is recorded in the type of the function. And this type is preferred over the type derived from
    // 'thisGT' if both are given.
    if (!funcGT.IsDefault()) {
        auto gt = tsManager->GetFuncThisGT(funcGT);
        if (!gt.IsDefault()) {
            return GateType(gt);
        }
    }
    return GateType(thisGT);
}

GateType TypeRecorder::TryGetNewTargetType(const TSManager *tsManager, GlobalTSTypeRef thisGT) const
{
    if (thisGT.IsDefault()) {
        return GateType::AnyType();
    }

    GateType thisType(thisGT);
    if (tsManager->IsClassInstanceTypeKind(thisType)) {
        return GateType(tsManager->GetClassType(thisGT));
    } else {
        return thisType;
    }
}

GateType TypeRecorder::TryGetFuncType(GlobalTSTypeRef funcGT) const
{
    if (funcGT.IsDefault()) {
        return GateType::AnyType();
    }
    return GateType(funcGT);
}

GateType TypeRecorder::GetType(const int32_t offset) const
{
    if (bcOffsetGtMap_.find(offset) != bcOffsetGtMap_.end()) {
        return bcOffsetGtMap_.at(offset);
    }
    return GateType::AnyType();
}

GateType TypeRecorder::GetArgType(const uint32_t argIndex) const
{
    ASSERT(argIndex < argTypes_.size());
    return argTypes_[argIndex];
}

GateType TypeRecorder::UpdateType(const int32_t offset, const GateType &type) const
{
    auto tempType = GetType(offset);
    if (!tempType.IsAnyType()) {
        ASSERT(type.IsAnyType());
        return tempType;
    }
    return type;
}

ElementsKind TypeRecorder::GetElementsKind(PGOSampleType type) const
{
    PGOHClassLayoutDesc *desc;
    if (type.IsProfileType() && decoder_->GetHClassLayoutDesc(type, &desc)) {
        auto elementsKind = desc->GetElementsKind();
        return elementsKind;
    }
    return ElementsKind::GENERIC;
}

PGOSampleType TypeRecorder::GetPGOHclassLayoutInfo(int32_t offset) const
{
    if (bcOffsetPGOOpTypeMap_.find(offset) != bcOffsetPGOOpTypeMap_.end()) {
        const auto iter = bcOffsetPGOOpTypeMap_.at(offset);
        if (iter.IsProfileType()) {
            PGOHClassLayoutDesc *desc;
            if (!decoder_->GetHClassLayoutDesc(iter, &desc)) {
                return PGOSampleType::NoneProfileType();
            }
        }
        return iter;
    }

    return PGOSampleType::NoneType();
}

PGOSampleType TypeRecorder::GetPGOTypeInfo(int32_t offset, EcmaOpcode opcode) const
{
    if (bcOffsetPGOOpTypeMap_.find(offset) == bcOffsetPGOOpTypeMap_.end()) {
        return PGOSampleType::NoneType();
    }
    switch (opcode) {
        case EcmaOpcode::GETITERATOR_IMM8:
        case EcmaOpcode::GETITERATOR_IMM16:
        case EcmaOpcode::CALLARG0_IMM8:
        case EcmaOpcode::CALLARG1_IMM8_V8:
        case EcmaOpcode::CALLARGS2_IMM8_V8_V8:
        case EcmaOpcode::CALLARGS3_IMM8_V8_V8_V8:
        case EcmaOpcode::CALLRANGE_IMM8_IMM8_V8:
        case EcmaOpcode::WIDE_CALLRANGE_PREF_IMM16_V8:
        case EcmaOpcode::CALLTHIS0_IMM8_V8:
        case EcmaOpcode::CALLTHIS1_IMM8_V8_V8:
        case EcmaOpcode::CALLTHIS2_IMM8_V8_V8_V8:
        case EcmaOpcode::CALLTHIS3_IMM8_V8_V8_V8_V8:
        case EcmaOpcode::CALLTHISRANGE_IMM8_IMM8_V8:
        case EcmaOpcode::WIDE_CALLTHISRANGE_PREF_IMM16_V8: {
            const auto sampleType = bcOffsetPGOOpTypeMap_.at(offset);
            ASSERT(sampleType.IsProfileType());
            if (!sampleType.GetProfileType().IsNativeFunctionId()) {
                return PGOSampleType::NoneType();
            }
            return sampleType;
        }
        default:
            break;
    }
    return GetPGOHclassLayoutInfo(offset);
}

GateType TypeRecorder::GetCallTargetType(int32_t offset) const
{
    if (bcOffsetCallTargetGtMap_.find(offset) != bcOffsetCallTargetGtMap_.end()) {
        return bcOffsetCallTargetGtMap_.at(offset);
    }
    return GateType::AnyType();
}

PGORWOpType TypeRecorder::GetRwOpType(int32_t offset) const
{
    if (bcOffsetPGORwTypeMap_.find(offset) != bcOffsetPGORwTypeMap_.end()) {
        return bcOffsetPGORwTypeMap_.at(offset);
    }
    return PGORWOpType();
}

std::vector<ElementsKind> TypeRecorder::LoadElementsKinds(int32_t offset) const
{
    std::vector<ElementsKind> elementsKinds;
    if (bcOffsetPGORwTypeMap_.find(offset) == bcOffsetPGORwTypeMap_.end()) {
        elementsKinds.emplace_back(ElementsKind::GENERIC);
        return elementsKinds;
    }

    PGORWOpType rwType = bcOffsetPGORwTypeMap_.at(offset);
    if (rwType.GetCount() == 0) {
        elementsKinds.emplace_back(ElementsKind::GENERIC);
        return elementsKinds;
    }
    for (uint32_t i = 0; i < rwType.GetCount(); i++) {
        PGOObjectInfo info = rwType.GetObjectInfo(i);
        auto profileType = info.GetProfileType();
        if (profileType.IsElementType()) {
            elementsKinds.emplace_back(ElementsKind(profileType.GetId()));
            continue;
        }
        PGOSampleType type(profileType);
        PGOHClassLayoutDesc *desc;
        if (!decoder_->GetHClassLayoutDesc(type, &desc)) {
            elementsKinds.emplace_back(ElementsKind::GENERIC);
            continue;
        }
        auto elementsKind = desc->GetElementsKind();
        elementsKinds.emplace_back(elementsKind);
    }

    // fiterate ElementsKind::None
    for (uint32_t i = 0; i < elementsKinds.size(); i++) {
        if (elementsKinds[i] == ElementsKind::NONE) {
            elementsKinds[i] = ElementsKind::GENERIC;
        }
    }

    return elementsKinds;
}

bool TypeRecorder::TypeNeedFilter(GlobalTSTypeRef gt) const
{
    return gt.IsDefault() || gt.IsGenericsModule();
}
}  // namespace panda::ecmascript
