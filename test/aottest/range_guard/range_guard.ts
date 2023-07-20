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
declare function print(arg:any):void;

function testcase1() {
    let testArray: Int32Array = new Int32Array([1, 2, 3]);
    let len: number = testArray.length;
    let a: number = testArray[0];
    let res: number = 5 + len;
    print(res)
}

function testcase2() {
    let testArray: Int32Array = new Int32Array([1, 2, 3]);
    let a: number = testArray[0];
    let len: number = testArray.length - 1;
    let res: number = 6 + len;
    print(res)
}

testcase1();  // 8
testcase2();  // 8