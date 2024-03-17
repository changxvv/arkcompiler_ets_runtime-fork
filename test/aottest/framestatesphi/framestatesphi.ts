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

declare function assert_equal(a: Object, b: Object):void;
class B {
    constructor() {
    }
    one = 1;
    choice = 0;
    g() : number {
        return 2;
    }
    f(x : boolean) {
        this.choice = x ? this.one : this.g();
    }
}

let b = new B()
assert_equal(b.choice, 0);
b.f(true);
assert_equal(b.choice, 1);
b.f(false);
assert_equal(b.choice, 2);