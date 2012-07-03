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

#ifndef _GRPADMINCREATEIOQCMD_H_
#define _GRPADMINCREATEIOQCMD_H_

#include "../group.h"

namespace GrpAdminCreateIOQCmd {


/**
 * Admin cmd set create IOSQ and create IOCQ test cases. Common group needed
 * to make the test cases more efficient where the same test may validate both
 * types of creation simultaneously.
 */
class GrpAdminCreateIOQCmd : public Group
{
public:
    GrpAdminCreateIOQCmd(size_t grpNum);
    virtual ~GrpAdminCreateIOQCmd();
};

}   // namespace

#endif
