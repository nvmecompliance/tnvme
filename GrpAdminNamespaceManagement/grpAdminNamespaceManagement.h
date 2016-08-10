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

#ifndef _GRPNAMESPACEMANAGEMENT_H_
#define _GRPNAMESPACEMANAGEMENT_H_

#include "../group.h"
#include "../Cmds/identify.h"

namespace GrpAdminNamespaceManagement {


/**
* This class implements admin cmd set async event request test cases.
*/
class GrpAdminNamespaceManagement : public Group
{
public:
    GrpAdminNamespaceManagement(size_t grpNum);
    virtual ~GrpAdminNamespaceManagement();
};

SharedIdentifyPtr CreateIdNamspcListActiveCmd(void);
SharedIdentifyPtr CreateIdNamspcListSubsystemCmd(void);
SharedIdentifyPtr CreateIdCtlrListAttachedCmd(void);
SharedIdentifyPtr CreateIdCtlrListSubsystemCmd(void);


struct NamespaceManagementCreateStruct {
    uint64_t    NSZE;
    uint64_t    NCAP;
    uint8_t     Reserved16_25[10];
    uint8_t     FLBAS;
    uint8_t     Reserved27_28[2];
    uint8_t     DPS;
    uint8_t     NMIC;
    uint8_t     Reserved31_383[353];
    /*
    NamespaceManagementCreateStruct(uint64_t NSZE, uint64_t NCAP, uint8_t FLBAS, uint8_t DPS, uint8_t NMIC) {
        this->NSZE  = NSZE;
        this->NCAP  = NCAP;
        this->FLBAS = FLBAS;
        this->DPS   = DPS;
        this->NMIC  = NMIC;
    }
    */
    /*
    NamespaceManagementCreateStruct(uint8_t *buffer) {
        this->NSZE  = (NamespaceManagementCreateStruct*) buffer->NSZE;
        this->NCAP  = (NamespaceManagementCreateStruct*) buffer->NCAP;
        this->FLBAS = (NamespaceManagementCreateStruct*) buffer->FLBAS;
        this->DPS   = (NamespaceManagementCreateStruct*) buffer->DPS;
        this->NMIC  = (NamespaceManagementCreateStruct*) buffer->NMIC;
    }
    */
    void print() {
        LOG_NRM("NamespaceManagementCreateStruct contains  NSZE = 0x%llx  NCAP = 0x%llx  FLBAS = 0x%x  DPS = 0x%x  NMIC = 0x%x",
                (long long unsigned) NSZE, (long long unsigned) NCAP, FLBAS, DPS, NMIC);
    }
} __attribute__((__packed__));

}   // namespace

#endif
