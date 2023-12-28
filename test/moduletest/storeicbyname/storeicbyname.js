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
 * @tc.name:storeicbyname
 * @tc.desc:test storeicbyname
 * @tc.type: FUNC
 * @tc.require: issueI7UTOA
 */

const arr = [];
for (let i = 0; i < 200; i++) {

}
for (let i = 0; i < 100; i++) {
    arr.length = 1025;
}
print("test successful !!!");
for (let i = 0; i < 100; i++) {
    const v77 = {};
    v77.__proto__ = null;
}
print("test set __proto__ null successful")
