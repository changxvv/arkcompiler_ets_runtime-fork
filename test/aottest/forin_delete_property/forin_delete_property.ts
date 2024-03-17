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

/*
 * @tc.name:forin_delete_property
 * @tc.desc:test forin_delete_property
 * @tc.type: FUNC
 * @tc.require: issueI89SMQ
 */

declare function print(str:any):string;
declare function assert_equal(a: Object, b: Object):void;
// fast path
let fast = {"a":1}
fast.b = "a"
var testArrary = ["a", "b"];
var j = 0;
for (let i in fast) {
    assert_equal(i, testArrary[j])
    j++
    delete fast.a
}
print("===============")
// slow path
let parent = {
    "c": undefined,
    "a": 1,
    "b": undefined,
    1: 2
}
let own = {
    "a": 1,
    "b": 1,
}
own.__proto__ = parent

testArrary = ["a", "b", "1"];
j = 0;
for (let i in own) {
    delete own.a
    assert_equal(i, testArrary[j])
    j++
    delete parent.c
}
print("===============")
testArrary = ["b", "1", "a"];
j = 0;
for (let i in own) {
    assert_equal(i, testArrary[j])
    j++
}