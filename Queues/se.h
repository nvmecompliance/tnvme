/*
 * Copyright (c) 2011, Intel Corporation.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef _SE_H_
#define _SE_H_

#include <stdint.h>


struct SEbyDW {
    uint32_t dw0;
    uint32_t dw1;
    uint32_t dw2;
    uint32_t dw3;
    uint32_t dw4;
    uint32_t dw5;
    uint32_t dw6;
    uint32_t dw7;
    uint32_t dw8;
    uint32_t dw9;
    uint32_t dw10;
    uint32_t dw11;
    uint32_t dw12;
    uint32_t dw13;
    uint32_t dw14;
    uint32_t dw15;
};

struct SEbyName {
    uint32_t OPC    : 8;
    uint32_t FUSE   : 2;
    uint32_t RES0   : 6;
    uint32_t CID    : 16;
    uint32_t NSID;
    uint32_t RES1;
    uint32_t RES2;
    uint32_t MPTRLo;
    uint32_t MPTRHi;
    uint32_t PRP1Lo;
    uint32_t PRP1Hi;
    uint32_t PRP2Lo;
    uint32_t PRP2Hi;
    uint32_t CDW10;
    uint32_t CDW11;
    uint32_t CDW12;
    uint32_t CDW13;
    uint32_t CDW14;
    uint32_t CDW15;
};

/**
 * Submission Element (SE) definition.
 */
union SE {
    struct SEbyDW   d;
    struct SEbyName n;
};


#endif
