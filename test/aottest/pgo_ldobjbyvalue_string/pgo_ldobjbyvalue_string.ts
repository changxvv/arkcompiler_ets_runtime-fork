/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

// @ts-nocheck
declare function print(arg:any):string;

function foo(a) {
    return a[0];
}


function test() {
    let a = "123456789012345678901234567890";
    let b = "123456789012345678901234567890";
    let c = a + b;
    foo(a);
    foo(c);
}

test();
ArkTools.printTypedOpProfilerAndReset("LOAD_ELEMENT");
