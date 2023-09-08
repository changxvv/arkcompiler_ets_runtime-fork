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

#include "ecmascript/pgo_profiler/types/pgo_profile_type.h"
#include "ecmascript/log_wrapper.h"
#include "ecmascript/pgo_profiler/pgo_profiler_info.h"

namespace panda::ecmascript::pgo {
ProfileTypeRef::ProfileTypeRef(PGOContext &context, const ProfileType &type)
{
    ApEntityId apId;
    context.GetProfileTypePool()->TryAdd(type, apId);
    UpdateId(apId);
}

ProfileType::ProfileType(PGOContext &context, ProfileTypeRef typeRef)
{
    if (!context.GetHeader()->SupportWideProfileType()) {
        ProfileTypeLegacy legacy(typeRef);
        UpdateId(legacy.GetId());
        UpdateKind(legacy.GetKind());
    } else {
        const auto *typeEntry = context.GetProfileTypePool()->GetRecord(typeRef.GetId());
        if (typeEntry == nullptr) {
            LOG_ECMA(ERROR) << "Profile type ref: " << typeRef.GetTypeString() << " not found in ap file.";
        } else {
            type_ = typeEntry->GetProfileType().GetRaw();
        }
    }
}
} // namespace panda::ecmascript::pgo