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

// === tests/cases/compiler/privacyInterface.ts ===
declare function AssertType(value:any, type:string):void;

export module m1 {
    export class C1_public {
        private f1() {
        }
    }


    class C2_private {
    }

    export interface C3_public {
        (c1: C1_public);
        (c1: C2_private);
        (): C1_public;
        (c2: number): C2_private;

        new (c1: C1_public);
        new (c1: C2_private);
        new (): C1_public;
        new (c2: number): C2_private;

        [c: number]: C1_public;
        [c: string]: C2_private;

        x: C1_public;
        y: C2_private;

        a?: C1_public;
        b?: C2_private;

        f1(a1: C1_public);
        f2(a1: C2_private);
        f3(): C1_public;
        f4(): C2_private;

    }

    interface C4_private {
        (c1: C1_public);
        (c1: C2_private);
        (): C1_public;
        (c2: number): C2_private;

        new (c1: C1_public);
        new (c1: C2_private);
        new (): C1_public;
        new (c2: number): C2_private;

        [c: number]: C1_public;
        [c: string]: C2_private;

        x: C1_public;
        y: C2_private;

        a?: C1_public;
        b?: C2_private;

        f1(a1: C1_public);
        f2(a1: C2_private);
        f3(): C1_public;
        f4(): C2_private;

    }
}


module m2 {
    export class C1_public {
        private f1() {
        }
    }


    class C2_private {
    }

    export interface C3_public {
        (c1: C1_public);
        (c1: C2_private);
        (): C1_public;
        (c2: number): C2_private;

        new (c1: C1_public);
        new (c1: C2_private);
        new (): C1_public;
        new (c2: number): C2_private;

        [c: number]: C1_public;
        [c: string]: C2_private;

        x: C1_public;
        y: C2_private;

        a?: C1_public;
        b?: C2_private;

        f1(a1: C1_public);
        f2(a1: C2_private);
        f3(): C1_public;
        f4(): C2_private;

    }

    interface C4_private {
        (c1: C1_public);
        (c1: C2_private);
        (): C1_public;
        (c2: number): C2_private;

        new (c1: C1_public);
        new (c1: C2_private);
        new (): C1_public;
        new (c2: number): C2_private;

        [c: number]: C1_public;
        [c: string]: C2_private;

        x: C1_public;
        y: C2_private;

        a?: C1_public;
        b?: C2_private;

        f1(a1: C1_public);
        f2(a1: C2_private);
        f3(): C1_public;
        f4(): C2_private;

    }
}

export class C5_public {
    private f1() {
    }
}


class C6_private {
}

export interface C7_public {
    (c1: C5_public);
    (c1: C6_private);
    (): C5_public;
    (c2: number): C6_private;

    new (c1: C5_public);
    new (c1: C6_private);
    new (): C5_public;
    new (c2: number): C6_private;

    [c: number]: C5_public;
    [c: string]: C6_private;

    x: C5_public;
    y: C6_private;

    a?: C5_public;
    b?: C6_private;

    f1(a1: C5_public);
    f2(a1: C6_private);
    f3(): C5_public;
    f4(): C6_private;

}

interface C8_private {
    (c1: C5_public);
    (c1: C6_private);
    (): C5_public;
    (c2: number): C6_private;

    new (c1: C5_public);
    new (c1: C6_private);
    new (): C5_public;
    new (c2: number): C6_private;

    [c: number]: C5_public;
    [c: string]: C6_private;

    x: C5_public;
    y: C6_private;

    a?: C5_public;
    b?: C6_private;

    f1(a1: C5_public);
    f2(a1: C6_private);
    f3(): C5_public;
    f4(): C6_private;

}

export module m3 {
    export interface m3_i_public {
        f1(): number;
    }

    interface m3_i_private {
        f2(): string;
    }

    interface m3_C1_private extends m3_i_public {
    }
    interface m3_C2_private extends m3_i_private {
    }
    export interface m3_C3_public extends m3_i_public {
    }
    export interface m3_C4_public extends m3_i_private {
    }

    interface m3_C5_private extends m3_i_private, m3_i_public {
    }
    export interface m3_C6_public extends  m3_i_private, m3_i_public {
    }
}


module m4 {
    export interface m4_i_public {
        f1(): number;
    }

    interface m4_i_private {
        f2(): string;
    }

    interface m4_C1_private extends m4_i_public {
    }
    interface m4_C2_private extends m4_i_private {
    }
    export interface m4_C3_public extends m4_i_public {
    }
    export interface m4_C4_public extends m4_i_private {
    }

    interface m4_C5_private extends m4_i_private, m4_i_public {
    }
    export interface m4_C6_public extends  m4_i_private, m4_i_public {
    }
}

export interface glo_i_public {
    f1(): number;
}

interface glo_i_private {
    f2(): string;
}

interface glo_C1_private extends glo_i_public {
}
interface glo_C2_private extends glo_i_private {
}
export interface glo_C3_public extends glo_i_public {
}
export interface glo_C4_public extends glo_i_private {
}

interface glo_C5_private extends glo_i_private, glo_i_public {
}
export interface glo_C6_public extends  glo_i_private, glo_i_public {
}

