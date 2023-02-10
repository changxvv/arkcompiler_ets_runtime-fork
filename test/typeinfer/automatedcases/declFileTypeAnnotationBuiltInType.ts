/*
* Copyright (c) Microsoft Corporation. All rights reserved.
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
*
* This file has been modified by Huawei to verify type inference by adding verification statements.
*/

// === tests/cases/compiler/declFileTypeAnnotationBuiltInType.ts ===
declare function AssertType(value:any, type:string):void;
// string
function foo(): string {
AssertType("", "string");
    return "";
}
function foo2() {
AssertType("", "string");
    return "";
}

// number
function foo3(): number {
AssertType(10, "int");
    return 10;
}
function foo4() {
AssertType(10, "int");
    return 10;
}

// boolean
function foo5(): boolean {
AssertType(true, "boolean");
    return true;
}
function foo6() {
AssertType(false, "boolean");
    return false;
}

// void
function foo7(): void {
    return;
}
function foo8() {
    return;
}

// any
function foo9(): any {
AssertType(undefined, "undefined");
    return undefined;
}
function foo10() {
AssertType(undefined, "undefined");
    return undefined;
}
