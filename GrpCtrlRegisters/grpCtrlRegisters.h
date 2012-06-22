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

#ifndef _GRPCTRLREGISTERS_H_
#define _GRPCTRLREGISTERS_H_

#include "../group.h"

namespace GrpCtrlRegisters {


/**
* This class implements a logical grouping of test cases for all NVME
* specification document releases. It is logically grouping the PCI BAR0/BAR1,
* i.e. all the NVME controller's registers address space for syntactical
* compliance.
*/
class GrpCtrlRegisters : public Group
{
public:
    GrpCtrlRegisters(size_t grpNum);
    virtual ~GrpCtrlRegisters();
};

}   // namespace

#endif
