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

#ifndef _GRPINFORMATIVE_H_
#define _GRPINFORMATIVE_H_

#include "../group.h"


/**
* This class implements a logical grouping of test cases for all NVME
* specification document releases. It is logically grouping information
* gathering duties. More specifically it is needed to gather any data
* someone would want to be dumped in order to help debug the reason for a
* test failure. These things could be the configuration of the controller
* or PCI address space, etc.
*/
class GrpInformative : public Group
{
public:
    GrpInformative(size_t grpNum, SpecRev specRev, ErrorRegs errRegs, int fd);
    virtual ~GrpInformative();
};


#endif
