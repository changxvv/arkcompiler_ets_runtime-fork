/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "ecmascript/jspandafile/panda_file_translator.h"

#include "ecmascript/file_loader.h"
#include "ecmascript/global_env.h"
#include "ecmascript/interpreter/interpreter.h"
#include "ecmascript/jspandafile/class_info_extractor.h"
#include "ecmascript/jspandafile/literal_data_extractor.h"
#include "ecmascript/jspandafile/js_pandafile_manager.h"
#include "ecmascript/jspandafile/program_object.h"
#include "ecmascript/js_array.h"
#include "ecmascript/js_function.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/tagged_array.h"
#include "ecmascript/ts_types/ts_manager.h"
#include "ecmascript/ts_types/ts_type_table.h"
#include "libpandabase/utils/utf.h"
#include "libpandafile/bytecode_instruction-inl.h"
#include "libpandafile/class_data_accessor-inl.h"

namespace panda::ecmascript {
template<class T, class... Args>
static T *InitializeMemory(T *mem, Args... args)
{
    return new (mem) T(std::forward<Args>(args)...);
}

void PandaFileTranslator::TranslateClasses(JSPandaFile *jsPandaFile, const CString &methodName)
{
    ASSERT(jsPandaFile != nullptr && jsPandaFile->GetMethods() != nullptr);
    JSMethod *methods = jsPandaFile->GetMethods();
    const panda_file::File *pf = jsPandaFile->GetPandaFile();
    size_t methodIdx = 0;
    panda_file::File::StringData sd = {static_cast<uint32_t>(methodName.size()),
                                       reinterpret_cast<const uint8_t *>(methodName.c_str())};
    std::set<const uint8_t *> translatedCode;
    Span<const uint32_t> classIndexes = jsPandaFile->GetClasses();
    for (const uint32_t index : classIndexes) {
        panda_file::File::EntityId classId(index);
        if (pf->IsExternal(classId)) {
            continue;
        }
        panda_file::ClassDataAccessor cda(*pf, classId);
        cda.EnumerateMethods([jsPandaFile, &translatedCode, &sd, methods, &methodIdx, pf]
            (panda_file::MethodDataAccessor &mda) {
            auto codeId = mda.GetCodeId();
            ASSERT(codeId.has_value());

            JSMethod *method = methods + (methodIdx++);
            panda_file::CodeDataAccessor codeDataAccessor(*pf, codeId.value());
            uint32_t codeSize = codeDataAccessor.GetCodeSize();

            uint32_t mainMethodIndex = jsPandaFile->GetMainMethodIndex();
            if (mainMethodIndex == 0 && pf->GetStringData(mda.GetNameId()) == sd) {
                jsPandaFile->UpdateMainMethodIndex(mda.GetMethodId().GetOffset());
            }

            InitializeMemory(method, jsPandaFile, mda.GetMethodId());
            method->SetHotnessCounter(EcmaInterpreter::METHOD_HOTNESS_THRESHOLD);
            method->InitializeCallField(codeDataAccessor.GetNumVregs(), codeDataAccessor.GetNumArgs());
            const uint8_t *insns = codeDataAccessor.GetInstructions();
            if (translatedCode.find(insns) == translatedCode.end()) {
                translatedCode.insert(insns);
                TranslateBytecode(jsPandaFile, codeSize, insns, method);
            }
            jsPandaFile->SetMethodToMap(method);
        });
    }
}

JSHandle<Program> PandaFileTranslator::GenerateProgram(EcmaVM *vm, const JSPandaFile *jsPandaFile)
{
    ObjectFactory *factory = vm->GetFactory();
    JSHandle<Program> program = factory->NewProgram();

    {
        JSThread *thread = vm->GetJSThread();
        EcmaHandleScope handleScope(thread);

        JSHandle<GlobalEnv> env = vm->GetGlobalEnv();

        uint32_t mainMethodIndex = jsPandaFile->GetMainMethodIndex();
        auto method = jsPandaFile->FindMethods(mainMethodIndex);
        ASSERT(method != nullptr);
        JSHandle<JSHClass> dynclass = JSHandle<JSHClass>::Cast(env->GetFunctionClassWithProto());
        JSHandle<JSFunction> mainFunc =
            factory->NewJSFunctionByDynClass(method, dynclass, FunctionKind::BASE_CONSTRUCTOR);

        program->SetMainFunction(thread, mainFunc.GetTaggedValue());

        JSTaggedValue constpool = vm->FindConstpool(jsPandaFile);
        if (constpool.IsHole()) {
            constpool = ParseConstPool(vm, jsPandaFile);
            vm->SetConstpool(jsPandaFile, constpool);
        }

        mainFunc->SetConstantPool(thread, constpool);
    }

    return program;
}

JSTaggedValue PandaFileTranslator::ParseConstPool(EcmaVM *vm, const JSPandaFile *jsPandaFile)
{
    JSThread *thread = vm->GetJSThread();
    JSHandle<GlobalEnv> env = vm->GetGlobalEnv();
    ObjectFactory *factory = vm->GetFactory();
    uint32_t constpoolIndex = jsPandaFile->GetConstpoolIndex();
    JSHandle<ConstantPool> constpool = factory->NewConstantPool(constpoolIndex + 1);
    const panda_file::File *pf = jsPandaFile->GetPandaFile();

    JSHandle<JSHClass> dynclass = JSHandle<JSHClass>::Cast(env->GetFunctionClassWithProto());
    JSHandle<JSHClass> normalDynclass = JSHandle<JSHClass>::Cast(env->GetFunctionClassWithoutProto());
    JSHandle<JSHClass> asyncDynclass = JSHandle<JSHClass>::Cast(env->GetAsyncFunctionClass());
    JSHandle<JSHClass> generatorDynclass = JSHandle<JSHClass>::Cast(env->GetGeneratorFunctionClass());
    JSHandle<JSHClass> asyncGeneratorDynclass = JSHandle<JSHClass>::Cast(env->GetAsyncGeneratorFunctionClass());

    const CUnorderedMap<uint32_t, uint64_t> &constpoolMap = jsPandaFile->GetConstpoolMap();
    const bool isLoadedAOT = jsPandaFile->IsLoadedAOT();
    auto fileLoader = vm->GetFileLoader();
    for (const auto &it : constpoolMap) {
        ConstPoolValue value(it.second);
        if (value.GetConstpoolType() == ConstPoolType::STRING) {
            panda_file::File::EntityId id(it.first);
            auto foundStr = pf->GetStringData(id);
            auto string = factory->GetRawStringFromStringTable(foundStr.data, foundStr.utf16_length, foundStr.is_ascii,
                                                               MemSpaceType::OLD_SPACE);
            constpool->Set(thread, value.GetConstpoolIndex(), JSTaggedValue(string));
        } else if (value.GetConstpoolType() == ConstPoolType::BASE_FUNCTION) {
            panda_file::File::EntityId id(it.first);
            auto method = jsPandaFile->FindMethods(it.first);
            ASSERT(method != nullptr);

            JSHandle<JSFunction> jsFunc = factory->NewJSFunctionByDynClass(
                method, dynclass, FunctionKind::BASE_CONSTRUCTOR, MemSpaceType::OLD_SPACE);
            if (isLoadedAOT) {
                fileLoader->SetAOTFuncEntry(jsPandaFile, jsFunc);
            }
            constpool->Set(thread, value.GetConstpoolIndex(), jsFunc.GetTaggedValue());
            jsFunc->SetConstantPool(thread, constpool.GetTaggedValue());
        } else if (value.GetConstpoolType() == ConstPoolType::NC_FUNCTION) {
            panda_file::File::EntityId id(it.first);
            auto method = jsPandaFile->FindMethods(it.first);
            ASSERT(method != nullptr);

            JSHandle<JSFunction> jsFunc = factory->NewJSFunctionByDynClass(
                method, normalDynclass, FunctionKind::NORMAL_FUNCTION, MemSpaceType::OLD_SPACE);
            if (isLoadedAOT) {
                fileLoader->SetAOTFuncEntry(jsPandaFile, jsFunc);
            }
            constpool->Set(thread, value.GetConstpoolIndex(), jsFunc.GetTaggedValue());
            jsFunc->SetConstantPool(thread, constpool.GetTaggedValue());
        } else if (value.GetConstpoolType() == ConstPoolType::GENERATOR_FUNCTION) {
            panda_file::File::EntityId id(it.first);
            auto method = jsPandaFile->FindMethods(it.first);
            ASSERT(method != nullptr);

            JSHandle<JSFunction> jsFunc =
                factory->NewJSFunctionByDynClass(method, generatorDynclass, FunctionKind::GENERATOR_FUNCTION);
            if (isLoadedAOT) {
                fileLoader->SetAOTFuncEntry(jsPandaFile, jsFunc);
            }
            // 26.3.4.3 prototype
            // Whenever a GeneratorFunction instance is created another ordinary object is also created and
            // is the initial value of the generator function's "prototype" property.
            JSHandle<JSTaggedValue> objFun = env->GetObjectFunction();
            JSHandle<JSObject> initialGeneratorFuncPrototype =
                factory->NewJSObjectByConstructor(JSHandle<JSFunction>(objFun), objFun);
            JSObject::SetPrototype(thread, initialGeneratorFuncPrototype, env->GetGeneratorPrototype());
            jsFunc->SetProtoOrDynClass(thread, initialGeneratorFuncPrototype);

            constpool->Set(thread, value.GetConstpoolIndex(), jsFunc.GetTaggedValue());
            jsFunc->SetConstantPool(thread, constpool.GetTaggedValue());
        } else if (value.GetConstpoolType() == ConstPoolType::ASYNC_GENERATOR_FUNCTION) {
            panda_file::File::EntityId id(it.first);
            auto method = jsPandaFile->FindMethods(it.first);
            ASSERT(method != nullptr);

            JSHandle<JSFunction> jsFunc =
                factory->NewJSFunctionByDynClass(method, asyncGeneratorDynclass,
                                                 FunctionKind::ASYNC_GENERATOR_FUNCTION);

            constpool->Set(thread, value.GetConstpoolIndex(), jsFunc.GetTaggedValue());
            jsFunc->SetConstantPool(thread, constpool.GetTaggedValue());
        } else if (value.GetConstpoolType() == ConstPoolType::ASYNC_FUNCTION) {
            panda_file::File::EntityId id(it.first);
            auto method = jsPandaFile->FindMethods(it.first);
            ASSERT(method != nullptr);

            JSHandle<JSFunction> jsFunc =
                factory->NewJSFunctionByDynClass(method, asyncDynclass, FunctionKind::ASYNC_FUNCTION);
            if (isLoadedAOT) {
                fileLoader->SetAOTFuncEntry(jsPandaFile, jsFunc);
            }
            constpool->Set(thread, value.GetConstpoolIndex(), jsFunc.GetTaggedValue());
            jsFunc->SetConstantPool(thread, constpool.GetTaggedValue());
        } else if (value.GetConstpoolType() == ConstPoolType::CLASS_FUNCTION) {
            panda_file::File::EntityId id(it.first);
            auto method = jsPandaFile->FindMethods(it.first);
            ASSERT(method != nullptr);
            JSHandle<ClassInfoExtractor> classInfoExtractor = factory->NewClassInfoExtractor(method);
            constpool->Set(thread, value.GetConstpoolIndex(), classInfoExtractor.GetTaggedValue());
        } else if (value.GetConstpoolType() == ConstPoolType::METHOD) {
            panda_file::File::EntityId id(it.first);
            auto method = jsPandaFile->FindMethods(it.first);
            ASSERT(method != nullptr);

            JSHandle<JSFunction> jsFunc = factory->NewJSFunctionByDynClass(
                method, normalDynclass, FunctionKind::NORMAL_FUNCTION, MemSpaceType::OLD_SPACE);
            if (isLoadedAOT) {
                fileLoader->SetAOTFuncEntry(jsPandaFile, jsFunc);
            }
            constpool->Set(thread, value.GetConstpoolIndex(), jsFunc.GetTaggedValue());
            jsFunc->SetConstantPool(thread, constpool.GetTaggedValue());
        } else if (value.GetConstpoolType() == ConstPoolType::OBJECT_LITERAL) {
            size_t index = it.first;
            JSMutableHandle<TaggedArray> elements(thread, JSTaggedValue::Undefined());
            JSMutableHandle<TaggedArray> properties(thread, JSTaggedValue::Undefined());
            LiteralDataExtractor::ExtractObjectDatas(thread, jsPandaFile, index, elements, properties);
            JSHandle<JSObject> obj = JSObject::CreateObjectFromProperties(thread, properties);
            if (isLoadedAOT) {
                fileLoader->SetAOTFuncEntryForLiteral(jsPandaFile, properties);
            }
            JSMutableHandle<JSTaggedValue> key(thread, JSTaggedValue::Undefined());
            JSMutableHandle<JSTaggedValue> valueHandle(thread, JSTaggedValue::Undefined());
            size_t elementsLen = elements->GetLength();
            for (size_t i = 0; i < elementsLen; i += 2) {  // 2: Each literal buffer contains a pair of key-value.
                key.Update(elements->Get(i));
                if (key->IsHole()) {
                    break;
                }
                valueHandle.Update(elements->Get(i + 1));
                JSObject::DefinePropertyByLiteral(thread, obj, key, valueHandle);
            }
            constpool->Set(thread, value.GetConstpoolIndex(), obj.GetTaggedValue());
        } else if (value.GetConstpoolType() == ConstPoolType::ARRAY_LITERAL) {
            size_t index = it.first;
            JSHandle<TaggedArray> literal =
                LiteralDataExtractor::GetDatasIgnoreType(thread, jsPandaFile, static_cast<size_t>(index));
            if (isLoadedAOT) {
                fileLoader->SetAOTFuncEntryForLiteral(jsPandaFile, literal);
            }
            uint32_t length = literal->GetLength();

            JSHandle<JSArray> arr(JSArray::ArrayCreate(thread, JSTaggedNumber(length)));
            arr->SetElements(thread, literal);
            constpool->Set(thread, value.GetConstpoolIndex(), arr.GetTaggedValue());
        } else if (value.GetConstpoolType() == ConstPoolType::CLASS_LITERAL) {
            size_t index = it.first;
            JSHandle<TaggedArray> literal =
                LiteralDataExtractor::GetDatasIgnoreType(thread, jsPandaFile, static_cast<size_t>(index));
            if (isLoadedAOT) {
                fileLoader->SetAOTFuncEntryForLiteral(jsPandaFile, literal);
            }
            constpool->Set(thread, value.GetConstpoolIndex(), literal.GetTaggedValue());
        }
    }

    // link JSPandaFile
    JSHandle<JSNativePointer> jsPandaFilePointer = factory->NewJSNativePointer(
        const_cast<JSPandaFile *>(jsPandaFile), JSPandaFileManager::RemoveJSPandaFile,
        JSPandaFileManager::GetInstance(), true);
    constpool->Set(thread, constpoolIndex, jsPandaFilePointer.GetTaggedValue());

    DefineClassesInConstPool(thread, constpool, jsPandaFile);

    return constpool.GetTaggedValue();
}

void PandaFileTranslator::FixOpcode(uint8_t *pc)
{
    auto opcode = static_cast<BytecodeInstruction::Opcode>(*pc);

    switch (opcode) {
        case BytecodeInstruction::Opcode::MOV_V4_V4:
            *pc = static_cast<uint8_t>(EcmaOpcode::MOV_V4_V4);
            break;
        case BytecodeInstruction::Opcode::MOV_DYN_V8_V8:
            *pc = static_cast<uint8_t>(EcmaOpcode::MOV_DYN_V8_V8);
            break;
        case BytecodeInstruction::Opcode::MOV_DYN_V16_V16:
            *pc = static_cast<uint8_t>(EcmaOpcode::MOV_DYN_V16_V16);
            break;
        case BytecodeInstruction::Opcode::LDA_STR_ID32:
            *pc = static_cast<uint8_t>(EcmaOpcode::LDA_STR_ID32);
            break;
        case BytecodeInstruction::Opcode::JMP_IMM8:
            *pc = static_cast<uint8_t>(EcmaOpcode::JMP_IMM8);
            break;
        case BytecodeInstruction::Opcode::JMP_IMM16:
            *pc = static_cast<uint8_t>(EcmaOpcode::JMP_IMM16);
            break;
        case BytecodeInstruction::Opcode::JMP_IMM32:
            *pc = static_cast<uint8_t>(EcmaOpcode::JMP_IMM32);
            break;
        case BytecodeInstruction::Opcode::JEQZ_IMM8:
            *pc = static_cast<uint8_t>(EcmaOpcode::JEQZ_IMM8);
            break;
        case BytecodeInstruction::Opcode::JEQZ_IMM16:
            *pc = static_cast<uint8_t>(EcmaOpcode::JEQZ_IMM16);
            break;
        case BytecodeInstruction::Opcode::JNEZ_IMM8:
            *pc = static_cast<uint8_t>(EcmaOpcode::JNEZ_IMM8);
            break;
        case BytecodeInstruction::Opcode::JNEZ_IMM16:
            *pc = static_cast<uint8_t>(EcmaOpcode::JNEZ_IMM16);
            break;
        case BytecodeInstruction::Opcode::LDA_DYN_V8:
            *pc = static_cast<uint8_t>(EcmaOpcode::LDA_DYN_V8);
            break;
        case BytecodeInstruction::Opcode::STA_DYN_V8:
            *pc = static_cast<uint8_t>(EcmaOpcode::STA_DYN_V8);
            break;
        case BytecodeInstruction::Opcode::LDAI_DYN_IMM32:
            *pc = static_cast<uint8_t>(EcmaOpcode::LDAI_DYN_IMM32);
            break;
        case BytecodeInstruction::Opcode::FLDAI_DYN_IMM64:
            *pc = static_cast<uint8_t>(EcmaOpcode::FLDAI_DYN_IMM64);
            break;
        case BytecodeInstruction::Opcode::RETURN_DYN:
            *pc = static_cast<uint8_t>(EcmaOpcode::RETURN_DYN);
            break;
        default:
            if (*pc != static_cast<uint8_t>(BytecodeInstruction::Opcode::ECMA_LDNAN_PREF_NONE)) {
                LOG_FULL(FATAL) << "Is not an Ecma Opcode opcode: " << static_cast<uint16_t>(opcode);
                UNREACHABLE();
            }
            *pc = *(pc + 1);
            *(pc + 1) = 0xFF;
            break;
    }
}

// reuse prefix 8bits to store slotid
void PandaFileTranslator::UpdateICOffset(JSMethod *method, uint8_t *pc)
{
    uint8_t offset = JSMethod::MAX_SLOT_SIZE;
    auto opcode = static_cast<EcmaOpcode>(*pc);
    switch (opcode) {
        case EcmaOpcode::TRYLDGLOBALBYNAME_PREF_ID32:
        case EcmaOpcode::TRYSTGLOBALBYNAME_PREF_ID32:
        case EcmaOpcode::LDGLOBALVAR_PREF_ID32:
        case EcmaOpcode::STGLOBALVAR_PREF_ID32:
        case EcmaOpcode::ADD2DYN_PREF_V8:
        case EcmaOpcode::SUB2DYN_PREF_V8:
        case EcmaOpcode::MUL2DYN_PREF_V8:
        case EcmaOpcode::DIV2DYN_PREF_V8:
        case EcmaOpcode::MOD2DYN_PREF_V8:
        case EcmaOpcode::SHL2DYN_PREF_V8:
        case EcmaOpcode::SHR2DYN_PREF_V8:
        case EcmaOpcode::ASHR2DYN_PREF_V8:
        case EcmaOpcode::AND2DYN_PREF_V8:
        case EcmaOpcode::OR2DYN_PREF_V8:
        case EcmaOpcode::XOR2DYN_PREF_V8:
        case EcmaOpcode::EQDYN_PREF_V8:
        case EcmaOpcode::NOTEQDYN_PREF_V8:
        case EcmaOpcode::LESSDYN_PREF_V8:
        case EcmaOpcode::LESSEQDYN_PREF_V8:
        case EcmaOpcode::GREATERDYN_PREF_V8:
        case EcmaOpcode::GREATEREQDYN_PREF_V8:
            offset = method->UpdateSlotSize(1);
            break;
        case EcmaOpcode::LDOBJBYVALUE_PREF_V8_V8:
        case EcmaOpcode::STOBJBYVALUE_PREF_V8_V8:
        case EcmaOpcode::STOWNBYVALUE_PREF_V8_V8:
        case EcmaOpcode::LDOBJBYNAME_PREF_ID32_V8:
        case EcmaOpcode::STOBJBYNAME_PREF_ID32_V8:
        case EcmaOpcode::STOWNBYNAME_PREF_ID32_V8:
        case EcmaOpcode::LDOBJBYINDEX_PREF_V8_IMM32:
        case EcmaOpcode::STOBJBYINDEX_PREF_V8_IMM32:
        case EcmaOpcode::STOWNBYINDEX_PREF_V8_IMM32:
        case EcmaOpcode::LDSUPERBYVALUE_PREF_V8_V8:
        case EcmaOpcode::STSUPERBYVALUE_PREF_V8_V8:
        case EcmaOpcode::LDSUPERBYNAME_PREF_ID32_V8:
        case EcmaOpcode::STSUPERBYNAME_PREF_ID32_V8:
        case EcmaOpcode::LDMODULEVAR_PREF_ID32_IMM8:
        case EcmaOpcode::STMODULEVAR_PREF_ID32:
            offset = method->UpdateSlotSize(2); // 2: occupy two ic slot
            break;
        default:
            return;
    }

    *(pc + 1) = offset;
}

void PandaFileTranslator::FixInstructionId32(const BytecodeInstruction &inst, uint32_t index,
                                             uint32_t fixOrder)
{
    // NOLINTNEXTLINE(hicpp-use-auto)
    auto pc = const_cast<uint8_t *>(inst.GetAddress());
    switch (inst.GetFormat()) {
        case BytecodeInstruction::Format::ID32: {
            uint8_t size = sizeof(uint32_t);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (memcpy_s(pc + FixInstructionIndex::FIX_ONE, size, &index, size) != EOK) {
                LOG_FULL(FATAL) << "memcpy_s failed";
                UNREACHABLE();
            }
            break;
        }
        case BytecodeInstruction::Format::PREF_ID16_IMM16_V8: {
            uint16_t u16Index = index;
            uint8_t size = sizeof(uint16_t);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (memcpy_s(pc + FixInstructionIndex::FIX_TWO, size, &u16Index, size) != EOK) {
                LOG_FULL(FATAL) << "memcpy_s failed";
                UNREACHABLE();
            }
            break;
        }
        case BytecodeInstruction::Format::PREF_ID32:
        case BytecodeInstruction::Format::PREF_ID32_V8:
        case BytecodeInstruction::Format::PREF_ID32_IMM8: {
            uint8_t size = sizeof(uint32_t);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (memcpy_s(pc + FixInstructionIndex::FIX_TWO, size, &index, size) != EOK) {
                LOG_FULL(FATAL) << "memcpy_s failed";
                UNREACHABLE();
            }
            break;
        }
        case BytecodeInstruction::Format::PREF_IMM16: {
            ASSERT(static_cast<uint16_t>(index) == index);
            uint16_t u16Index = index;
            uint8_t size = sizeof(uint16_t);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (memcpy_s(pc + FixInstructionIndex::FIX_TWO, size, &u16Index, size) != EOK) {
                LOG_FULL(FATAL) << "memcpy_s failed";
                UNREACHABLE();
            }
            break;
        }
        case BytecodeInstruction::Format::PREF_ID16_IMM16_IMM16_V8_V8: {
            // Usually, we fix one part of instruction one time. But as for instruction DefineClassWithBuffer,
            // which use both method id and literal buffer id.Using fixOrder indicates fix Location.
            if (fixOrder == 0) {
                uint8_t size = sizeof(uint16_t);
                ASSERT(static_cast<uint16_t>(index) == index);
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                if (memcpy_s(pc + FixInstructionIndex::FIX_TWO, size, &index, size) != EOK) {
                    LOG_FULL(FATAL) << "memcpy_s failed";
                    UNREACHABLE();
                }
                break;
            }
            if (fixOrder == 1) {
                ASSERT(static_cast<uint16_t>(index) == index);
                uint16_t u16Index = index;
                uint8_t size = sizeof(uint16_t);
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                if (memcpy_s(pc + FixInstructionIndex::FIX_FOUR, size, &u16Index, size) != EOK) {
                    LOG_FULL(FATAL) << "memcpy_s failed";
                    UNREACHABLE();
                }
                break;
            }
            break;
        }
        default:
            UNREACHABLE();
    }
}

void PandaFileTranslator::TranslateBytecode(JSPandaFile *jsPandaFile, uint32_t insSz, const uint8_t *insArr,
                                            const JSMethod *method)
{
    const panda_file::File *pf = jsPandaFile->GetPandaFile();
    auto bcIns = BytecodeInstruction(insArr);
    auto bcInsLast = bcIns.JumpTo(insSz);
    while (bcIns.GetAddress() != bcInsLast.GetAddress()) {
        if (bcIns.HasFlag(BytecodeInstruction::Flags::STRING_ID) &&
            BytecodeInstruction::HasId(bcIns.GetFormat(), 0)) {
            auto index = jsPandaFile->GetOrInsertConstantPool(
                ConstPoolType::STRING, bcIns.GetId().AsFileId().GetOffset());
            FixInstructionId32(bcIns, index);
        } else {
            BytecodeInstruction::Opcode opcode = static_cast<BytecodeInstruction::Opcode>(bcIns.GetOpcode());
            switch (opcode) {
                uint32_t index;
                uint32_t methodId;
                case BytecodeInstruction::Opcode::ECMA_DEFINEFUNCDYN_PREF_ID16_IMM16_V8:
                    methodId = pf->ResolveMethodIndex(method->GetMethodId(), bcIns.GetId().AsIndex()).GetOffset();
                    index = jsPandaFile->GetOrInsertConstantPool(ConstPoolType::BASE_FUNCTION, methodId);
                    FixInstructionId32(bcIns, index);
                    break;
                case BytecodeInstruction::Opcode::ECMA_DEFINENCFUNCDYN_PREF_ID16_IMM16_V8:
                    methodId = pf->ResolveMethodIndex(method->GetMethodId(), bcIns.GetId().AsIndex()).GetOffset();
                    index = jsPandaFile->GetOrInsertConstantPool(ConstPoolType::NC_FUNCTION, methodId);
                    FixInstructionId32(bcIns, index);
                    break;
                case BytecodeInstruction::Opcode::ECMA_DEFINEGENERATORFUNC_PREF_ID16_IMM16_V8:
                    methodId = pf->ResolveMethodIndex(method->GetMethodId(), bcIns.GetId().AsIndex()).GetOffset();
                    index = jsPandaFile->GetOrInsertConstantPool(ConstPoolType::GENERATOR_FUNCTION, methodId);
                    FixInstructionId32(bcIns, index);
                    break;
                case BytecodeInstruction::Opcode::ECMA_DEFINEASYNCGENERATORFUNC_PREF_ID16_IMM16_V8:
                    methodId = pf->ResolveMethodIndex(method->GetMethodId(), bcIns.GetId().AsIndex()).GetOffset();
                    index = jsPandaFile->GetOrInsertConstantPool(ConstPoolType::ASYNC_GENERATOR_FUNCTION, methodId);
                    FixInstructionId32(bcIns, index);
                    break;
                case BytecodeInstruction::Opcode::ECMA_DEFINEASYNCFUNC_PREF_ID16_IMM16_V8:
                    methodId = pf->ResolveMethodIndex(method->GetMethodId(), bcIns.GetId().AsIndex()).GetOffset();
                    index = jsPandaFile->GetOrInsertConstantPool(ConstPoolType::ASYNC_FUNCTION, methodId);
                    FixInstructionId32(bcIns, index);
                    break;
                case BytecodeInstruction::Opcode::ECMA_DEFINEMETHOD_PREF_ID16_IMM16_V8:
                    methodId = pf->ResolveMethodIndex(method->GetMethodId(), bcIns.GetId().AsIndex()).GetOffset();
                    index = jsPandaFile->GetOrInsertConstantPool(ConstPoolType::METHOD, methodId);
                    FixInstructionId32(bcIns, index);
                    break;
                case BytecodeInstruction::Opcode::ECMA_CREATEOBJECTWITHBUFFER_PREF_IMM16:
                case BytecodeInstruction::Opcode::ECMA_CREATEOBJECTHAVINGMETHOD_PREF_IMM16: {
                    auto imm = bcIns.GetImm<BytecodeInstruction::Format::PREF_IMM16>();
                    index = jsPandaFile->GetOrInsertConstantPool(ConstPoolType::OBJECT_LITERAL,
                        static_cast<uint16_t>(imm));
                    FixInstructionId32(bcIns, index);
                    break;
                }
                case BytecodeInstruction::Opcode::ECMA_CREATEARRAYWITHBUFFER_PREF_IMM16: {
                    auto imm = bcIns.GetImm<BytecodeInstruction::Format::PREF_IMM16>();
                    index = jsPandaFile->GetOrInsertConstantPool(ConstPoolType::ARRAY_LITERAL,
                        static_cast<uint16_t>(imm));
                    FixInstructionId32(bcIns, index);
                    break;
                }
                case BytecodeInstruction::Opcode::ECMA_DEFINECLASSWITHBUFFER_PREF_ID16_IMM16_IMM16_V8_V8: {
                    methodId = pf->ResolveMethodIndex(method->GetMethodId(), bcIns.GetId().AsIndex()).GetOffset();
                    index = jsPandaFile->GetOrInsertConstantPool(ConstPoolType::CLASS_FUNCTION, methodId);
                    FixInstructionId32(bcIns, index);
                    auto imm = bcIns.GetImm<BytecodeInstruction::Format::PREF_ID16_IMM16_IMM16_V8_V8>();
                    index = jsPandaFile->GetOrInsertConstantPool(ConstPoolType::CLASS_LITERAL,
                        static_cast<uint16_t>(imm));
                    FixInstructionId32(bcIns, index, 1);
                    break;
                }
                default:
                    break;
            }
        }
        // NOLINTNEXTLINE(hicpp-use-auto)
        auto pc = const_cast<uint8_t *>(bcIns.GetAddress());
        bcIns = bcIns.GetNext();
        FixOpcode(pc);
        UpdateICOffset(const_cast<JSMethod *>(method), pc);
    }
}

void PandaFileTranslator::DefineClassesInConstPool(JSThread *thread, JSHandle<ConstantPool> constpool,
                                                   const JSPandaFile *jsPandaFile)
{
    uint32_t length = constpool->GetLength();
    uint32_t index = 0;
    const bool isLoadedAOT = jsPandaFile->IsLoadedAOT();
    auto fileLoader = thread->GetEcmaVM()->GetFileLoader();
    while (index < length - 1) {
        JSTaggedValue value = constpool->Get(index);
        if (!value.IsClassInfoExtractor()) {
            index++;
            continue;
        }

        // Here, using a law: when inserting ctor in index of constantpool, the index + 1 location will be inserted by
        // corresponding class literal. Because translator fixes ECMA_DEFINECLASSWITHBUFFER two consecutive times.
        JSTaggedValue nextValue = constpool->Get(index + 1);
        ASSERT(nextValue.IsTaggedArray());

        JSHandle<ClassInfoExtractor> extractor(thread, value);
        JSHandle<TaggedArray> literal(thread, nextValue);
        ClassInfoExtractor::BuildClassInfoExtractorFromLiteral(thread, extractor, literal);
        JSHandle<JSFunction> cls = ClassHelper::DefineClassTemplate(thread, extractor, constpool);
        if (isLoadedAOT) {
            fileLoader->SetAOTFuncEntry(jsPandaFile, cls);
        }
        constpool->Set(thread, index, cls);
        index += 2;  // 2: pair of extractor and literal
    }
}
}  // namespace panda::ecmascript
