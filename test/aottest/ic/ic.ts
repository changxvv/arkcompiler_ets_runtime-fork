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

declare function print(a:any):string;

class C {
    x:number;
    constructor() {
        this.x = 123;
    }
}

let c1 = new C();
print(c1.x);

let c2 = new C();
print(c2.x);


function foo(p) {
    return p[1]
}

let a = [1, 2]
let b = [1, 2.1, 3]
for (let i = 0; i < 100000; i++) {
    var a1 = foo(a)
    var a2 = foo(b);
}
print(a1)
print(a2)

let o1 = [1, , 2.5]
for (var i = 1; i < 10000; i++) {
  o1 = [1, , 2.5]
}
print(o1);