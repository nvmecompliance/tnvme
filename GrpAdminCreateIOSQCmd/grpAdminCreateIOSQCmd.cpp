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

#include "grpAdminCreateIOSQCmd.h"
#include "../Exception/frmwkEx.h"
#include "createResources_r10b.h"
#include "unsupportRsvdFields_r10b.h"
#include "invalidQID_r10b.h"
#include "maxQSizeExceed_r10b.h"
#include "completionQInvalid_r10b.h"
#include "acceptQPriority_r10b.h"

namespace GrpAdminCreateIOSQCmd {


GrpAdminCreateIOSQCmd::GrpAdminCreateIOSQCmd(size_t grpNum) :
    Group(grpNum, "GrpAdminCreateIOSQCmd",
        "Admin cmd set create IOSQ test cases")
{
    // For complete details about the APPEND_TEST_AT_?LEVEL() macros:
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering" and
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Strategy
    switch (gCmdLine.rev) {
    case SPECREV_10b:
        APPEND_TEST_AT_XLEVEL(CreateResources_r10b, GrpAdminCreateIOSQCmd)
        APPEND_TEST_AT_YLEVEL(UnsupportRsvdFields_r10b, GrpAdminCreateIOSQCmd)
        APPEND_TEST_AT_YLEVEL(MaxQSizeExceed_r10b, GrpAdminCreateIOSQCmd)
        APPEND_TEST_AT_YLEVEL(CompletionQInvalid_r10b, GrpAdminCreateIOSQCmd)
        APPEND_TEST_AT_XLEVEL(InvalidQID_r10b, GrpAdminCreateIOSQCmd)
        APPEND_TEST_AT_XLEVEL(AcceptQPriority_r10b, GrpAdminCreateIOSQCmd)
        break;

    default:
    case SPECREVTYPE_FENCE:
        throw FrmwkEx(HERE, "Object created with an unknown SpecRev=%d",
            gCmdLine.rev);
    }
}


GrpAdminCreateIOSQCmd::~GrpAdminCreateIOSQCmd()
{
    // mTests deallocated in parent
}

}   // namespace
