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

// === tests/cases/compiler/privacyClass.ts ===
declare function AssertType(value:any, type:string):void;

export module m1 {
    export interface m1_i_public {
    }

    interface m1_i_private {
    }

    export class m1_c_public {
        private f1() {
        }
    }

    class m1_c_private {
    }

    class m1_C1_private extends m1_c_public {
    }
    class m1_C2_private extends m1_c_private {
    }
    export class m1_C3_public extends m1_c_public {
    }
    export class m1_C4_public extends m1_c_private {
    }

    class m1_C5_private implements m1_i_public {
    }
    class m1_C6_private implements m1_i_private {
    }
    export class m1_C7_public implements m1_i_public {
    }
    export class m1_C8_public implements m1_i_private {
    }

    class m1_C9_private extends m1_c_public implements m1_i_private, m1_i_public {
    }
    class m1_C10_private extends m1_c_private implements  m1_i_private, m1_i_public {
    }
    export class m1_C11_public extends m1_c_public implements  m1_i_private, m1_i_public {
    }
    export class m1_C12_public extends m1_c_private implements  m1_i_private, m1_i_public {
    }
}


module m2 {
    export interface m2_i_public {
    }

    interface m2_i_private {
    }

    export class m2_c_public {
        private f1() {
        }
    }

    class m2_c_private {
    }

    class m2_C1_private extends m2_c_public {
    }
    class m2_C2_private extends m2_c_private {
    }
    export class m2_C3_public extends m2_c_public {
    }
    export class m2_C4_public extends m2_c_private {
    }

    class m2_C5_private implements m2_i_public {
    }
    class m2_C6_private implements m2_i_private {
    }
    export class m2_C7_public implements m2_i_public {
    }
    export class m2_C8_public implements m2_i_private {
    }

    class m2_C9_private extends m2_c_public implements m2_i_private, m2_i_public {
    }
    class m2_C10_private extends m2_c_private implements  m2_i_private, m2_i_public {
    }
    export class m2_C11_public extends m2_c_public implements  m2_i_private, m2_i_public {
    }
    export class m2_C12_public extends m2_c_private implements  m2_i_private, m2_i_public {
    }
}

export interface glo_i_public {
}

interface glo_i_private {
}

export class glo_c_public {
    private f1() {
    }
}

class glo_c_private {
}

class glo_C1_private extends glo_c_public {
}
class glo_C2_private extends glo_c_private {
}
export class glo_C3_public extends glo_c_public {
}
export class glo_C4_public extends glo_c_private {
}

class glo_C5_private implements glo_i_public {
}
class glo_C6_private implements glo_i_private {
}
export class glo_C7_public implements glo_i_public {
}
export class glo_C8_public implements glo_i_private {
}

class glo_C9_private extends glo_c_public implements glo_i_private, glo_i_public {
}
class glo_C10_private extends glo_c_private implements  glo_i_private, glo_i_public {
}
export class glo_C11_public extends glo_c_public implements  glo_i_private, glo_i_public {
}
export class glo_C12_public extends glo_c_private implements  glo_i_private, glo_i_public {
}

