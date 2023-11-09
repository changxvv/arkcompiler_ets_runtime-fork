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
 * @tc.name:stringSlice
 * @tc.desc:test String.slice
 * @tc.type: FUNC
 * @tc.require: issueI5NO8G
 */

let str1 = "a";
let str2 = "c";
print(str1.localeCompare(str2));

let str3 = "check";
let str4 = "against";
print(str3.localeCompare(str4));

let str5 = "abc";
let str6 = "abc";
print(str5.localecCompare(str6));
