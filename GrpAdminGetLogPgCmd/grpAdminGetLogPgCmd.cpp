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

#include "grpAdminGetLogPgCmd.h"
#include "createResources_r10b.h"
#include "prp1_r10b.h"
#include "prp1PRP2_r10b.h"
#include "unsupportRrvdFields_r10b.h"
#include "numDIsAdhered_r10b.h"
#include "invalidLogPageNVMSet_r10b.h"
#include "invalidNamspc_r10b.h"
#include "mandatoryErrInfo_r10b.h"
#include "mandatorySMART_r10b.h"
#include "mandatoryFW_r10b.h"


namespace GrpAdminGetLogPgCmd {


GrpAdminGetLogPgCmd::GrpAdminGetLogPgCmd(size_t grpNum) :
    Group(grpNum, "GrpAdminGetLogPgCmd",
        "Admin cmd set get log page test cases")
{
    // For complete details about the APPEND_TEST_AT_?LEVEL() macros:
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering" and
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Strategy
    switch (gCmdLine.rev) {
    case SPECREV_10b:
        APPEND_TEST_AT_XLEVEL(CreateResources_r10b, GrpAdminGetLogPgCmd)
        APPEND_TEST_AT_YLEVEL(PRP1_r10b, GrpAdminGetLogPgCmd)
        APPEND_TEST_AT_YLEVEL(PRP1PRP2_r10b, GrpAdminGetLogPgCmd)
        APPEND_TEST_AT_YLEVEL(UnsupportRrvdFields_r10b, GrpAdminGetLogPgCmd)
        APPEND_TEST_AT_YLEVEL(NUMDIsAdhered_r10b, GrpAdminGetLogPgCmd)
        APPEND_TEST_AT_YLEVEL(InvalidLogPageNVMSet_r10b, GrpAdminGetLogPgCmd)
        APPEND_TEST_AT_YLEVEL(InvalidNamspc_r10b, GrpAdminGetLogPgCmd)
        APPEND_TEST_AT_YLEVEL(MandatoryErrInfo_r10b, GrpAdminGetLogPgCmd)
        APPEND_TEST_AT_YLEVEL(MandatorySMART_r10b, GrpAdminGetLogPgCmd)
        APPEND_TEST_AT_YLEVEL(MandatoryFW_r10b, GrpAdminGetLogPgCmd)
        break;

    default:
    case SPECREVTYPE_FENCE:
        throw FrmwkEx(HERE, "Object created with an unknown SpecRev=%d",
            gCmdLine.rev);
    }
}


GrpAdminGetLogPgCmd::~GrpAdminGetLogPgCmd()
{
    // mTests deallocated in parent
}

}   // namespace
