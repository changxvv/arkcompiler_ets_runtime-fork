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
declare function print(arg:any, arg1?:any):string;

let str:string = "Hello World";

// two const
print("Hello" + "World");
print(str + "Hello");
print("" + "" + "123" + "");

// one const
let strs = ["OD", "Huawei", ""];
for (let i = 0; i<3; ++i) {
    let m:string = strs[i];
    print("Hello,  " + m);
}

// no const
function foo(flag) {
    let str = ["BUG", "full of bug"];
    if (flag) {
        return str[0];
    } else {
        return str[1];
    }
}
let left:string = foo(true);
let right1:string = foo(true);
let right2:string = foo(false);
print(left + right1);
print(left + right2);
print(right2 + right2);

