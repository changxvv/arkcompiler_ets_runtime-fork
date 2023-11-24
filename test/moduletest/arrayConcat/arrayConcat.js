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
const array1 = ['a', 'b', 'c'];
const array2 = ['d', 'e', 'f'];
const array3 = array1.concat(array2);
print(array3);

const letters = ["a", "b", "c"];
const numbers = [1, 2, 3];

const alphaNumeric = letters.concat(numbers);
print(alphaNumeric);

const num1 = [1, 2, 3];
const num2 = [4, 5, 6];
const num3 = [7, 8, 9];

const numbers1 = num1.concat(num2, num3);

print(numbers1);


const letters1 = ["a", "b", "c"];

const alphaNumeric1 = letters1.concat(1, [2, 3]);

print(alphaNumeric1);

const num11 = [[1]];
const num22 = [2, [3]];

const numbers2 = num1.concat(num22);

print(numbers2);
// [[1], 2, [3]]

num11[0].push(4);

print(numbers2);

