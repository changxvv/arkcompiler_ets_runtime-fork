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

#ifndef ECMASCRIPT_LOG_WRAPPER_H
#define ECMASCRIPT_LOG_WRAPPER_H

#include "ecmascript/log.h"

#define LOG_FULL(level) LOG_ECMA(level) << __func__ << ":" << __LINE__ << " "
#define LOG_GC(level) LOG_ECMA(level) << " [gc]: "
#define LOG_INTERPRETER(level) LOG_ECMA(level) << " [interpreter]: "
#define LOG_COMPILER(level) LOG_ECMA(level) << " [compiler]: "
#define LOG_DEBUGGER(level) LOG_ECMA(level) << " [debugger]: "
#define LOG_ECMA_IF(cond, level) (cond) && LOG_ECMA(level)

#endif  // ECMASCRIPT_LOG_WRAPPER_H
