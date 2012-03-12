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

#include "../Exception/frmwkEx.h"
#include "grpCtrlRegisters.h"
#include "allCtrlRegs_r10b.h"
#include "ctrlrResetDefaults_r10b.h"

namespace GrpCtrlRegisters {


GrpCtrlRegisters::GrpCtrlRegisters(size_t grpNum, SpecRev specRev,
    ErrorRegs errRegs, int fd) :
    Group(grpNum, specRev, "Controller registers syntactic")
{
    // ------------------------CHANGE NOTICE: (3-2-2012)------------------------
    // The rule to keep groups and tests at a well known constant reference
    // number for all of time is to restrictive. A new scheme has replaced
    // that strategy. For complete details refer to:
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering" and
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Strategy
    switch (mSpecRev) {
    case SPECREV_10b:
        APPEND_TEST_AT_XLEVEL(AllCtrlRegs_r10b, fd, GrpCtrlRegisters, errRegs)
        APPEND_TEST_AT_XLEVEL(CtrlrResetDefaults_r10b, fd, GrpCtrlRegisters, errRegs)
        break;

    default:
    case SPECREVTYPE_FENCE:
        throw FrmwkEx("Object created with an unknown SpecRev=%d", specRev);
    }
}


GrpCtrlRegisters::~GrpCtrlRegisters()
{
    // mTests deallocated in parent
}

}   // namespace
