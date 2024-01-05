/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
 * @tc.name:proxy
 * @tc.desc:test Proxy
 * @tc.type: FUNC
 * @tc.require: issueI5NO8G
 */
function add(a, b) {
    return a + b
}

let addProxy = new Proxy(add, {
    apply: function(target, thisObj, args) {
        return target.apply(thisObj, args)
    }
})

for (var a = 1; a < 4; a++) {
    print(addProxy(a, 12345 * a) == add(12345 * a, a))
}

class EmployeeEntity {
}
let obj = new Proxy(EmployeeEntity,{});
let arr = [obj];
let set = new Set(arr);
print(set.has(obj));

const a1 = [-122934378, 536870889, -4294967295, -8, 11];
const a2 = {
};
const a3 = new Proxy(a1, a2);
Object.freeze(a3);
print(Object.isFrozen(a3));

// CreateObjectWithExcludedKeysDealWithProxy
const v0 = {
    value : 'name'
};
const v5 = new Proxy(v0, {});
const {...v1} = v5;
print(v5.value);
print(v1.value);
print(v0.value);
const arr2 = [1, 2, 3];
const handler = {};
const proxy = new Proxy(arr2, handler);
const arr3 = proxy.constructor(1, 2, 3, 4);
print(arr3);

function Target() {}
var handler1 = {
    construct: function(t, args) {
        return new t(args[0], args[1]);
    }
};
var P = new Proxy(Target, handler1);
new P(1, 1.1);
print(JSON.stringify(P));

try {
    const v0 = /\011+/iyd;
    const v2 = new Uint8ClampedArray(v0, Uint8ClampedArray, Uint8ClampedArray);
    const v5 = new Proxy({}, {});
    new v5(...v2);
} catch (e) {
    print("constructor is fail");
}
