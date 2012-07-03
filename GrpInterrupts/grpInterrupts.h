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

#ifndef _GRPINTERRUPTS_H_
#define _GRPINTERRUPTS_H_

#include "../group.h"

namespace GrpInterrupts {


/**
* This class implements a logical grouping of test cases for basic
* initialization of NVME hardware. This mainly consists of the operations
* one needs during a system power up to get the hardware operational.
*/
class GrpInterrupts : public Group
{
public:
    GrpInterrupts(size_t grpNum);
    virtual ~GrpInterrupts();
};

}   // namespace

#endif
