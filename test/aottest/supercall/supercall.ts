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

declare function print(str:any):string;

class A {
    name:string;
    value:number;
    constructor(name:string, value:number) {
        this.name = name;
        this.value = value;
    }
}
class B extends A {
    constructor(name:string, value:number) {
        super(name, value);
    }
}

var obj = new B("AOT", 123);
print(obj.name);
print(obj.value);

class M {
    constructor(value) {
        this.stageValue = value;
    }
}
class BM extends M {
    constructor(value) {
        super(value);
    }
}

let mdf = new M(2);
print(mdf.stageValue);
for (let i = 0; i < 12; i++) {
    mdf = new BM(9);
}
print(mdf.stageValue);