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

declare function print(arg:any):string;

function foo(array, flag) {
    array.length;
    if (flag < 0) {
        array[0]; // only Uint8Array type
    } else {
        array[0]; // only Array type
    }
}
let a = new Uint8Array([1,2]);
let a2 = [1,2,3];
foo(a2, 1);
foo(a, -1);
print("run success");
