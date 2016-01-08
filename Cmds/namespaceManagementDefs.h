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

#ifndef _NAMESPACEMANAGEMENTDEFS_H_
#define _NAMESPACEMANAGEMENTDEFS_H_


struct NamspcMgmtCreateStruct {
    IdNamespcStructNonVS  IdNamspc;
    uint8_t               Resvd[640];
    uint8_t               VS[3072];
} __attribute__((__packed__));

typedef enum NamspcMgmtSelectBits {
    NAMSPC_MGMT_SEL_CREATE  =   0x0,
    NAMSPC_MGMT_SEL_DELETE  =   0x1,
} NamspcMgmtSelectBits;


#endif
