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

#include "ecmascript/jspandafile/program_object.h"

namespace panda::ecmascript {
JSHandle<ConstantPool> ConstantPool::GetDeserializedConstantPool(EcmaVM *vm, const JSPandaFile *jsPandaFile,
                                                                 int32_t cpID)
{
    return JSHandle<ConstantPool>(vm->GetJSThread()->GetCurrentEcmaContext()->GetAOTFileManager()->GetDeserializedConstantPool(jsPandaFile, cpID));
}
}