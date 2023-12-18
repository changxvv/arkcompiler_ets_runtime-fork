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

// === tests/cases/conformance/parser/ecmascript5/parserSbp_7.9_A9_T3.ts ===
declare function AssertType(value:any, type:string):void;
// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/**
 * Check Do-While Statement for automatic semicolon insertion
 *
 * @path bestPractice/Sbp_7.9_A9_T3.js
 * @description Execute do { \n ; \n }while(false) true
 */

//CHECK#1
do {
  ;
} while (false) true
AssertType(true, "boolean");



