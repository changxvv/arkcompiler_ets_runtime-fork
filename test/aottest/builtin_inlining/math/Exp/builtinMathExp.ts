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

declare interface ArkTools {
    isAOTCompiled(args: any): boolean;
}
declare function print(arg:any):string;

function printZero(x: any) {
    if (Object.is(x, -0)) {
        print("-0");
    } else {
        print(x);
    }
}

function replace(a : number)
{
    return a;
}

// Use try to prevent inlining to main
function printExp(x: any) {
    try {
        print(Math.exp(x));
    } finally {
    }
}

let doubleObj = {
    valueOf: () => { return 2.7; }
}

let nanObj = {
    valueOf: () => { return "something"; }
}

let obj = {
    valueOf: () => {
        print("obj.valueOf")
        return -23;
    }
};

// Check without params
print(Math.exp()); // NaN

// Check with single param
print(Math.exp(0)); // 1
print(Math.exp(-0)); // 1
print(Math.exp(1)); // 2.718281828459045
print(Math.exp(-100)); // 3.720075976020836e-44
print(Math.exp(100)); // 2.6881171418161356e+43
print(Math.exp(10e-10)); // 1.000000001

// Check with special float params
printZero(Math.exp(-Infinity)); // 0
print(Math.exp(Infinity)); // Infinity
print(Math.exp(NaN)); // NaN

// Check with 2 params
print(Math.exp(1, 1)); // 2.718281828459045

// Check with 3 params
print(Math.exp(1, 1, 1)); // 2.718281828459045

// Check with 4 params
print(Math.exp(1, 1, 1, 1)); // 2.718281828459045

// Check with 5 params
print(Math.exp(1, 1, 1, 1, 1)); // 2.718281828459045

try {
    print(Math.exp(1)); // 2.718281828459045
} catch(e) {}

// Replace standart builtin
let trueExp = Math.exp
Math.exp = replace
print(Math.exp(111)); // 111
Math.exp = trueExp

print(Math.exp(1)); // 2.718281828459045

// Call standart builtin with non-number param
// Check Type: NotNumber1
printExp("1"); // 2.718281828459045
// Check Type: NotNumber1
printExp("NaN"); // NaN
// Check Type: NotNumber1
printExp("abc"); // NaN

if (ArkTools.isAOTCompiled(printExp)) {
    // Replace standard builtin after call to standard builtin was profiled
    Math.exp = replace
}

// Check Type: NotCallTarget1
printExp(1); // 2.718281828459045
// Check Type: NotCallTarget1
printExp(2); // 7.38905609893065
// Check Type: NotCallTarget1
printExp("1"); // 2.718281828459045
// Check Type: NotCallTarget1
printExp("2"); // 7.38905609893065

Math.exp = trueExp

// Check IR correctness inside try-block
try {
    print(Math.exp(1)); //: 2.718281828459045
    print(Math.exp(1, 2)); //: 2.718281828459045
    printExp(1, 2); //: 2.718281828459045
    // Check Type: NotNumber1
    printExp("abc", 3e3); //: NaN
} catch (e) {
}

// Check Type: NotNumber1
printExp(obj); // 1.026187963170189e-10
// Check Type: NotNumber1
printExp(doubleObj); // 14.879731724872837
// Check Type: NotNumber1
printExp(nanObj); // NaN