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

#ifndef _GRPNVMWRITEREADCOMBO_H_
#define _GRPNVMWRITEREADCOMBO_H_

#include "../group.h"
#include "../Exception/frmwkEx.h"


namespace GrpNVMWriteReadCombo {


/**
* This class implements NVM cmd set write/read test cases that require data
* verification to detect a successful outcome.
*/
class GrpNVMWriteReadCombo : public Group
{
public:
    GrpNVMWriteReadCombo(size_t grpNum);
    virtual ~GrpNVMWriteReadCombo();
};

}   // namespace

#endif
