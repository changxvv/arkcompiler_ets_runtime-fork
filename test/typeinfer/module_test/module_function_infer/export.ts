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

declare function AssertType(value:any, type:string):void;

export function foo1 (v:number, s:number):number {
    return v + s;
}

export function foo2 (s:number):number {
    return s;
}

AssertType(foo1, "(number, number) => number");
AssertType(foo1(123, 456), "number");

AssertType(foo2, "(number) => number");
AssertType(foo2(123), "number");

export let foo3 = foo1(123, 456) + foo2(123);
