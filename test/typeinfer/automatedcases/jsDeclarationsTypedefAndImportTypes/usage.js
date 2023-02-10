/*
* Copyright (c) Microsoft Corporation. All rights reserved.
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
*
* This file has been modified by Huawei to verify type inference by adding verification statements.
*/

// === tests/cases/conformance/jsdoc/declarations/usage.js ===
declare function AssertType(value:any, type:string):void;
/**
 * @typedef {import("./conn")} Conn
 */

class Wrap {
    /**
     * @param {Conn} c
     */
    constructor(c) {
        this.connItem = c.item;
AssertType(this.connItem = c.item, "number");
AssertType(this.connItem, "any");
AssertType(this, "this");
AssertType(c.item, "number");

        /** @type {import("./conn").Whatever} */
        this.another = "";
AssertType(this.another = "", "string");
AssertType(this.another, "import("tests/cases/conformance/jsdoc/declarations/conn").Whatever");
AssertType(this, "this");
AssertType("", "string");
    }
}

module.exports = {
AssertType(module.exports = {    Wrap}, "typeof module.exports");
AssertType(module.exports, "typeof module.exports");
AssertType({    Wrap}, "{ Wrap: typeof Wrap; }");

    Wrap
AssertType(Wrap, "typeof Wrap");

};

