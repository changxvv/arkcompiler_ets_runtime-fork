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

[
    Float64Array,
    Float32Array,
    Int32Array,
    Int16Array,
    Int8Array,
    Uint32Array,
    Uint16Array,
    Uint8Array,
    Uint8ClampedArray
].forEach(function (ctor, i) {
    testTypeArrayAt1(ctor)
});

function testTypeArrayAt1(ctor) {
    let result = [];
    let obj = new ctor([10, 11, 12, 13, 14]);
    let result1 = obj.subarray(0, 5);
    print(result1);
    result1 = obj.subarray(-1, 1);
    print(result1.length);
    result1 = obj.subarray(2, 10);
    print(result1);
    result1 = obj.subarray(2, -10);
    print(result1.length);
}

var v0 = new Uint8Array(64);
try {
  v0 = v0.subarray(64);
} catch (e) {
  print(e);
}
try {
  v0 = v0.subarray(64);
} catch (e) {
  print(e);
}
print(v0);
