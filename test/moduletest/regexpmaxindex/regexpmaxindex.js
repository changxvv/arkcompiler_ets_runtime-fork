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

// mjsunit/regress/regress-6209.js
function testAdvanceStringIndex(lastIndex, expectedLastIndex) {
    let exec_count = 0;
    let last_last_index = -1;
  
    let fake_re = {
        exec: () => { return (exec_count++ == 0) ? [""] : null },
        get lastIndex() { return lastIndex; },
        set lastIndex(value) { last_last_index = value },
        get global() { return true; },
        get flags() { return "g"; }
    };
  
    if ("" != RegExp.prototype[Symbol.match].call(fake_re, "abc")) {
        print("test failed");
    }
    if (expectedLastIndex != last_last_index) {
        print("test failed");
    }
}

testAdvanceStringIndex(new Number(42), 43);  // Value wrapper.
testAdvanceStringIndex(ArkTools.allocateHeapNumber(), 1);  // HeapNumber.
testAdvanceStringIndex(4294967295, 4294967296);  // HeapNumber.
