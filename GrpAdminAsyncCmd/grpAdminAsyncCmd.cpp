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

#include "grpAdminAsyncCmd.h"
#include "unsupportRrvdFields_r10b.h"
#include "abortByReset_r10b.h"
#include "verifyMaxEvents_r10b.h"
#include "verifyMasking_r10b.h"
#include "verifyEventQueueing_r10b.h"


namespace GrpAdminAsyncCmd {


GrpAdminAsyncCmd::GrpAdminAsyncCmd(size_t grpNum) :
    Group(grpNum, "GrpAdminAsyncCmd", "Admin cmd set async event request test cases.")
{
    // For complete details about the APPEND_TEST_AT_?LEVEL() macros:
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering" and
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Strategy
    switch (gCmdLine.rev) {
    case SPECREV_10b:
        APPEND_TEST_AT_XLEVEL(UnsupportRrvdFields_r10b, GrpAdminAsyncCmd)
        APPEND_TEST_AT_XLEVEL(AbortByReset_r10b, GrpAdminAsyncCmd)
        APPEND_TEST_AT_XLEVEL(VerifyMaxEvents_r10b, GrpAdminAsyncCmd)
        APPEND_TEST_AT_XLEVEL(VerifyMasking_r10b, GrpAdminAsyncCmd)
        APPEND_TEST_AT_XLEVEL(VerifyEventQueueing_r10b, GrpAdminAsyncCmd)
        break;

    default:
    case SPECREVTYPE_FENCE:
        throw FrmwkEx(HERE, "Object created with an unknown SpecRev=%d",
            gCmdLine.rev);
    }
}


GrpAdminAsyncCmd::~GrpAdminAsyncCmd()
{
    // mTests deallocated in parent
}

}   // namespace
