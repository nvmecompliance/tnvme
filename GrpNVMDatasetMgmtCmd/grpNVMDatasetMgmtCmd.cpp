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

#include "grpNVMDatasetMgmtCmd.h"
#include "../Exception/frmwkEx.h"
#include "identify_r10b.h"
#include "createResources_r10b.h"
#include "invalidNamspc_r10b.h"
#include "unsupportRsvdFields_r10b.h"
#include "prp1PRP2NR_r10b.h"
#include "attributes_r10b.h"

namespace GrpNVMDatasetMgmtCmd {


GrpNVMDatasetMgmtCmd::GrpNVMDatasetMgmtCmd(size_t grpNum, SpecRev specRev,
    ErrorRegs errRegs, int fd) :
    Group(grpNum, specRev, "GrpNVMDatasetMgmtCmd",
        "NVM cmd set data set mgmt tests")
{
    // For complete details about the APPEND_TEST_AT_?LEVEL() macros:
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering" and
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Strategy
    switch (mSpecRev) {
    case SPECREV_10b:
        APPEND_TEST_AT_XLEVEL(Identify_r10b, fd, GrpNVMDatasetMgmtCmd, errRegs)
        APPEND_TEST_AT_XLEVEL(CreateResources_r10b, fd, GrpNVMDatasetMgmtCmd, errRegs)
        APPEND_TEST_AT_YLEVEL(InvalidNamspc_r10b, fd, GrpNVMDatasetMgmtCmd, errRegs)
        APPEND_TEST_AT_YLEVEL(UnsupportRsvdFields_r10b, fd, GrpNVMDatasetMgmtCmd, errRegs)
        APPEND_TEST_AT_YLEVEL(PRP1PRP2NR_r10b, fd, GrpNVMDatasetMgmtCmd, errRegs)
        APPEND_TEST_AT_YLEVEL(Attributes_r10b, fd, GrpNVMDatasetMgmtCmd, errRegs)
        break;

    default:
    case SPECREVTYPE_FENCE:
        throw FrmwkEx(HERE, "Object created with an unknown SpecRev=%d", specRev);
    }
}


GrpNVMDatasetMgmtCmd::~GrpNVMDatasetMgmtCmd()
{
    // mTests deallocated in parent
}

}   // namespace
