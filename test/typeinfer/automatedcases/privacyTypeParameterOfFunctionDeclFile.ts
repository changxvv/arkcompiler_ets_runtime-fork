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

// === tests/cases/compiler/privacyTypeParameterOfFunctionDeclFile.ts ===
declare function AssertType(value:any, type:string):void;

class privateClass {
}

export class publicClass {
}

export interface publicInterfaceWithPrivateTypeParameters {
    new <T extends privateClass>(): privateClass;  // Error
    <T extends privateClass>(): privateClass;  // Error
    myMethod<T extends privateClass>(): privateClass;  // Error
}

export interface publicInterfaceWithPublicTypeParameters {
    new <T extends publicClass>(): publicClass;
    <T extends publicClass>(): publicClass;
    myMethod<T extends publicClass>(): publicClass;
}

interface privateInterfaceWithPrivateTypeParameters {
    new <T extends privateClass>(): privateClass;
    <T extends privateClass>(): privateClass;
    myMethod<T extends privateClass>(): privateClass;
}

interface privateInterfaceWithPublicTypeParameters {
    new <T extends publicClass>(): publicClass;
    <T extends publicClass>(): publicClass;
    myMethod<T extends publicClass>(): publicClass;
}

export class publicClassWithWithPrivateTypeParameters {
    static myPublicStaticMethod<T extends privateClass>() {  // Error
    }
    private static myPrivateStaticMethod<T extends privateClass>() {
    }
    myPublicMethod<T extends privateClass>() {  // Error
    }
    private myPrivateMethod<T extends privateClass>() {
    }
}

export class publicClassWithWithPublicTypeParameters {
    static myPublicStaticMethod<T extends publicClass>() {
    }
    private static myPrivateStaticMethod<T extends publicClass>() { 
    }
    myPublicMethod<T extends publicClass>() {
    }
    private myPrivateMethod<T extends publicClass>() {
    }
}

class privateClassWithWithPrivateTypeParameters {
    static myPublicStaticMethod<T extends privateClass>() {
    }
    private static myPrivateStaticMethod<T extends privateClass>() {
    }
    myPublicMethod<T extends privateClass>() {
    }
    private myPrivateMethod<T extends privateClass>() {
    }
}

class privateClassWithWithPublicTypeParameters {
    static myPublicStaticMethod<T extends publicClass>() {
    }
    private static myPrivateStaticMethod<T extends publicClass>() {
    }
    myPublicMethod<T extends publicClass>() {
    }
    private myPrivateMethod<T extends publicClass>() {
    }
}

export function publicFunctionWithPrivateTypeParameters<T extends privateClass>() {  // Error
}

export function publicFunctionWithPublicTypeParameters<T extends publicClass>() {
}

function privateFunctionWithPrivateTypeParameters<T extends privateClass>() {
}

function privateFunctionWithPublicTypeParameters<T extends publicClass>() {
}

export interface publicInterfaceWithPublicTypeParametersWithoutExtends {
    new <T>(): publicClass;
    <T>(): publicClass;
    myMethod<T>(): publicClass;
}

interface privateInterfaceWithPublicTypeParametersWithoutExtends {
    new <T>(): publicClass;
    <T>(): publicClass;
    myMethod<T>(): publicClass;
}

export class publicClassWithWithPublicTypeParametersWithoutExtends {
    static myPublicStaticMethod<T>() {
    }
    private static myPrivateStaticMethod<T>() {
    }
    myPublicMethod<T>() {
    }
    private myPrivateMethod<T>() {
    }
}
class privateClassWithWithPublicTypeParametersWithoutExtends {
    static myPublicStaticMethod<T>() {
    }
    private static myPrivateStaticMethod<T>() {
    }
    myPublicMethod<T>() {
    }
    private myPrivateMethod<T>() {
    }
}

export function publicFunctionWithPublicTypeParametersWithoutExtends<T>() {
}

function privateFunctionWithPublicTypeParametersWithoutExtends<T>() {
}

export interface publicInterfaceWithPrivatModuleTypeParameters {
    new <T extends privateModule.publicClass>(): privateModule.publicClass;  // Error
    <T extends privateModule.publicClass>(): privateModule.publicClass;  // Error
    myMethod<T extends privateModule.publicClass>(): privateModule.publicClass;  // Error
}
export class publicClassWithWithPrivateModuleTypeParameters {
    static myPublicStaticMethod<T extends privateModule.publicClass>() {  // Error
    }
    myPublicMethod<T extends privateModule.publicClass>() {  // Error
    }
}
export function publicFunctionWithPrivateMopduleTypeParameters<T extends privateModule.publicClass>() {  // Error
}


interface privateInterfaceWithPrivatModuleTypeParameters {
    new <T extends privateModule.publicClass>(): privateModule.publicClass;
    <T extends privateModule.publicClass>(): privateModule.publicClass;
    myMethod<T extends privateModule.publicClass>(): privateModule.publicClass;
}
class privateClassWithWithPrivateModuleTypeParameters {
    static myPublicStaticMethod<T extends privateModule.publicClass>() {
    }
    myPublicMethod<T extends privateModule.publicClass>() {
    }
}
function privateFunctionWithPrivateMopduleTypeParameters<T extends privateModule.publicClass>() {
}


export module publicModule {
    class privateClass {
    }

    export class publicClass {
    }

    export interface publicInterfaceWithPrivateTypeParameters {
        new <T extends privateClass>(): privateClass;  // Error
        <T extends privateClass>(): privateClass;  // Error
        myMethod<T extends privateClass>(): privateClass;  // Error
    }

    export interface publicInterfaceWithPublicTypeParameters {
        new <T extends publicClass>(): publicClass;
        <T extends publicClass>(): publicClass;
        myMethod<T extends publicClass>(): publicClass;
    }

    interface privateInterfaceWithPrivateTypeParameters {
        new <T extends privateClass>(): privateClass;
        <T extends privateClass>(): privateClass;
        myMethod<T extends privateClass>(): privateClass;
    }

    interface privateInterfaceWithPublicTypeParameters {
        new <T extends publicClass>(): publicClass;
        <T extends publicClass>(): publicClass;
        myMethod<T extends publicClass>(): publicClass;
    }

    export class publicClassWithWithPrivateTypeParameters {
        static myPublicStaticMethod<T extends privateClass>() {  // Error
        }
        private static myPrivateStaticMethod<T extends privateClass>() {
        }
        myPublicMethod<T extends privateClass>() {  // Error
        }
        private myPrivateMethod<T extends privateClass>() {
        }
    }

    export class publicClassWithWithPublicTypeParameters {
        static myPublicStaticMethod<T extends publicClass>() {
        }
        private static myPrivateStaticMethod<T extends publicClass>() {
        }
        myPublicMethod<T extends publicClass>() {
        }
        private myPrivateMethod<T extends publicClass>() {
        }
    }

    class privateClassWithWithPrivateTypeParameters {
        static myPublicStaticMethod<T extends privateClass>() {
        }
        private static myPrivateStaticMethod<T extends privateClass>() {
        }
        myPublicMethod<T extends privateClass>() {
        }
        private myPrivateMethod<T extends privateClass>() {
        }
    }

    class privateClassWithWithPublicTypeParameters {
        static myPublicStaticMethod<T extends publicClass>() {
        }
        private static myPrivateStaticMethod<T extends publicClass>() {
        }
        myPublicMethod<T extends publicClass>() {
        }
        private myPrivateMethod<T extends publicClass>() {
        }
    }

    export function publicFunctionWithPrivateTypeParameters<T extends privateClass>() {  // Error
    }

    export function publicFunctionWithPublicTypeParameters<T extends publicClass>() {
    }

    function privateFunctionWithPrivateTypeParameters<T extends privateClass>() {
    }

    function privateFunctionWithPublicTypeParameters<T extends publicClass>() {
    }

    export interface publicInterfaceWithPublicTypeParametersWithoutExtends {
        new <T>(): publicClass;
        <T>(): publicClass;
        myMethod<T>(): publicClass;
    }

    interface privateInterfaceWithPublicTypeParametersWithoutExtends {
        new <T>(): publicClass;
        <T>(): publicClass;
        myMethod<T>(): publicClass;
    }

    export class publicClassWithWithPublicTypeParametersWithoutExtends {
        static myPublicStaticMethod<T>() {
        }
        private static myPrivateStaticMethod<T>() {
        }
        myPublicMethod<T>() {
        }
        private myPrivateMethod<T>() {
        }
    }
    class privateClassWithWithPublicTypeParametersWithoutExtends {
        static myPublicStaticMethod<T>() {
        }
        private static myPrivateStaticMethod<T>() {
        }
        myPublicMethod<T>() {
        }
        private myPrivateMethod<T>() {
        }
    }

    export function publicFunctionWithPublicTypeParametersWithoutExtends<T>() {
    }

    function privateFunctionWithPublicTypeParametersWithoutExtends<T>() {
    }

    export interface publicInterfaceWithPrivatModuleTypeParameters {
        new <T extends privateModule.publicClass>(): privateModule.publicClass;  // Error
        <T extends privateModule.publicClass>(): privateModule.publicClass;  // Error
        myMethod<T extends privateModule.publicClass>(): privateModule.publicClass;  // Error
    }
    export class publicClassWithWithPrivateModuleTypeParameters {
        static myPublicStaticMethod<T extends privateModule.publicClass>() {  // Error
        }
        myPublicMethod<T extends privateModule.publicClass>() {  // Error
        }
    }
    export function publicFunctionWithPrivateMopduleTypeParameters<T extends privateModule.publicClass>() {  // Error
    }


    interface privateInterfaceWithPrivatModuleTypeParameters {
        new <T extends privateModule.publicClass>(): privateModule.publicClass;  
        <T extends privateModule.publicClass>(): privateModule.publicClass;  
        myMethod<T extends privateModule.publicClass>(): privateModule.publicClass;  
    }
    class privateClassWithWithPrivateModuleTypeParameters {
        static myPublicStaticMethod<T extends privateModule.publicClass>() {  
        }
        myPublicMethod<T extends privateModule.publicClass>() { 
        }
    }
    function privateFunctionWithPrivateMopduleTypeParameters<T extends privateModule.publicClass>() { 
    }

}

module privateModule {
    class privateClass {
    }

    export class publicClass {
    }

    export interface publicInterfaceWithPrivateTypeParameters {
        new <T extends privateClass>(): privateClass; 
        <T extends privateClass>(): privateClass;
        myMethod<T extends privateClass>(): privateClass; 
    }

    export interface publicInterfaceWithPublicTypeParameters {
        new <T extends publicClass>(): publicClass;
        <T extends publicClass>(): publicClass;
        myMethod<T extends publicClass>(): publicClass;
    }

    interface privateInterfaceWithPrivateTypeParameters {
        new <T extends privateClass>(): privateClass;
        <T extends privateClass>(): privateClass;
        myMethod<T extends privateClass>(): privateClass;
    }

    interface privateInterfaceWithPublicTypeParameters {
        new <T extends publicClass>(): publicClass;
        <T extends publicClass>(): publicClass;
        myMethod<T extends publicClass>(): publicClass;
    }

    export class publicClassWithWithPrivateTypeParameters {
        static myPublicStaticMethod<T extends privateClass>() { 
        }
        private static myPrivateStaticMethod<T extends privateClass>() {
        }
        myPublicMethod<T extends privateClass>() {  
        }
        private myPrivateMethod<T extends privateClass>() {
        }
    }

    export class publicClassWithWithPublicTypeParameters {
        static myPublicStaticMethod<T extends publicClass>() {
        }
        private static myPrivateStaticMethod<T extends publicClass>() {
        }
        myPublicMethod<T extends publicClass>() {
        }
        private myPrivateMethod<T extends publicClass>() {
        }
    }

    class privateClassWithWithPrivateTypeParameters {
        static myPublicStaticMethod<T extends privateClass>() {
        }
        private static myPrivateStaticMethod<T extends privateClass>() {
        }
        myPublicMethod<T extends privateClass>() {
        }
        private myPrivateMethod<T extends privateClass>() {
        }
    }

    class privateClassWithWithPublicTypeParameters {
        static myPublicStaticMethod<T extends publicClass>() {
        }
        private static myPrivateStaticMethod<T extends publicClass>() {
        }
        myPublicMethod<T extends publicClass>() {
        }
        private myPrivateMethod<T extends publicClass>() {
        }
    }

    export function publicFunctionWithPrivateTypeParameters<T extends privateClass>() { 
    }

    export function publicFunctionWithPublicTypeParameters<T extends publicClass>() {
    }

    function privateFunctionWithPrivateTypeParameters<T extends privateClass>() {
    }

    function privateFunctionWithPublicTypeParameters<T extends publicClass>() {
    }

    export interface publicInterfaceWithPublicTypeParametersWithoutExtends {
        new <T>(): publicClass;
        <T>(): publicClass;
        myMethod<T>(): publicClass;
    }

    interface privateInterfaceWithPublicTypeParametersWithoutExtends {
        new <T>(): publicClass;
        <T>(): publicClass;
        myMethod<T>(): publicClass;
    }

    export class publicClassWithWithPublicTypeParametersWithoutExtends {
        static myPublicStaticMethod<T>() {
        }
        private static myPrivateStaticMethod<T>() {
        }
        myPublicMethod<T>() {
        }
        private myPrivateMethod<T>() {
        }
    }
    class privateClassWithWithPublicTypeParametersWithoutExtends {
        static myPublicStaticMethod<T>() {
        }
        private static myPrivateStaticMethod<T>() {
        }
        myPublicMethod<T>() {
        }
        private myPrivateMethod<T>() {
        }
    }

    export function publicFunctionWithPublicTypeParametersWithoutExtends<T>() {
    }

    function privateFunctionWithPublicTypeParametersWithoutExtends<T>() {
    }
}

