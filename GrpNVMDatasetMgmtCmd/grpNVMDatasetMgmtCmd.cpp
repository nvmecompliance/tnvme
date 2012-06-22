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
#include "createResources_r10b.h"
#include "invalidNamspc_r10b.h"
#include "unsupportRsvdFields_r10b.h"
#include "prp1PRP2NR_r10b.h"
#include "attributes_r10b.h"
#include "verifyNUSE_r10b.h"

namespace GrpNVMDatasetMgmtCmd {


GrpNVMDatasetMgmtCmd::GrpNVMDatasetMgmtCmd(size_t grpNum) :
    Group(grpNum, "GrpNVMDatasetMgmtCmd", "NVM cmd set data set mgmt tests")
{
    // For complete details about the APPEND_TEST_AT_?LEVEL() macros:
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering" and
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Strategy
    switch (gCmdLine.rev) {
    case SPECREV_10b:
        APPEND_TEST_AT_XLEVEL(CreateResources_r10b, GrpNVMDatasetMgmtCmd)
        APPEND_TEST_AT_YLEVEL(InvalidNamspc_r10b, GrpNVMDatasetMgmtCmd)
        APPEND_TEST_AT_YLEVEL(UnsupportRsvdFields_r10b, GrpNVMDatasetMgmtCmd)
        APPEND_TEST_AT_YLEVEL(PRP1PRP2NR_r10b, GrpNVMDatasetMgmtCmd)
        APPEND_TEST_AT_YLEVEL(Attributes_r10b, GrpNVMDatasetMgmtCmd)
        APPEND_TEST_AT_YLEVEL(VerifyNUSE_r10b, GrpNVMDatasetMgmtCmd)
        break;

    default:
    case SPECREVTYPE_FENCE:
        throw FrmwkEx(HERE, "Object created with an unknown SpecRev=%d",
            gCmdLine.rev);
    }
}


GrpNVMDatasetMgmtCmd::~GrpNVMDatasetMgmtCmd()
{
    // mTests deallocated in parent
}

}   // namespace
