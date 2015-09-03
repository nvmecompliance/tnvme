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

#ifndef _FEATUREDEFS_H_
#define _FEATUREDEFS_H_


typedef enum ErrRecoveryDW11Bits {
    ER_TLER         = 0x0000FFFF,
    ER_DULBE        = 0x00010000,
    ER_RESV         = 0xFFFE0000
} ErrRecoveryDW11Bits;

#endif
