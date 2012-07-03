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


GrpCtrlRegisters::GrpCtrlRegisters(size_t grpNum) :
    Group(grpNum, "GrpCtrlRegisters", "Controller registers syntactic")
{
    // For complete details about the APPEND_TEST_AT_?LEVEL() macros:
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering" and
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Strategy
    switch (gCmdLine.rev) {
    case SPECREV_10b:
        APPEND_TEST_AT_XLEVEL(AllCtrlRegs_r10b, GrpCtrlRegisters)
        APPEND_TEST_AT_XLEVEL(CtrlrResetDefaults_r10b, GrpCtrlRegisters)
        break;

    default:
    case SPECREVTYPE_FENCE:
        throw FrmwkEx(HERE, "Object created with an unknown SpecRev=%d",
            gCmdLine.rev);
    }
}


GrpCtrlRegisters::~GrpCtrlRegisters()
{
    // mTests deallocated in parent
}

}   // namespace
