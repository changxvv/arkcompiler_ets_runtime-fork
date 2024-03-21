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

declare function print(arg:any):string;
function replace(a : number)
{
    return a;
}

let len:number = 1;

// Check without params
len = Math.cosh();
print(len); // NaN

len = Math.cosh(NaN);
print(len); // NaN

// Check with single param
len = Math.cosh(0);
print(len); // 1 

// Check with single param
len = Math.cosh(-1);
print(len); // 1.5430806348152437

// Check with single param
len = Math.cosh(1);
print(len); // 1.5430806348152437

// Check with single param
len = Math.cosh(10);
print(len); // 11013.232920103324

// Replace standart builtin
let true_cosh = Math.cosh
Math.cosh = replace
len = Math.cosh(111);
print(len);

// Call standart builtin with non-number param
Math.cosh = true_cosh
len = Math.cosh("NaN"); // deopt
print(len); // NaN