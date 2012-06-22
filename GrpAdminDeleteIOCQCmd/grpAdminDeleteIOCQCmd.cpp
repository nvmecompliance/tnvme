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

#include "grpAdminDeleteIOCQCmd.h"
#include "../Exception/frmwkEx.h"
#include "createResources_r10b.h"
#include "unsupportRsvdFields_r10b.h"
#include "invalidQID_r10b.h"
#include "deleteAllAtOnce_r10b.h"
#include "deleteFullQ_r10b.h"

namespace GrpAdminDeleteIOCQCmd {


GrpAdminDeleteIOCQCmd::GrpAdminDeleteIOCQCmd(size_t grpNum) :
    Group(grpNum, "GrpAdminDeleteIOCQCmd",
        "Admin cmd set delete IOCQ test cases")
{
    // For complete details about the APPEND_TEST_AT_?LEVEL() macros:
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering" and
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Strategy
    switch (gCmdLine.rev) {
    case SPECREV_10b:
        APPEND_TEST_AT_XLEVEL(CreateResources_r10b, GrpAdminDeleteIOCQCmd)
        APPEND_TEST_AT_YLEVEL(UnsupportRsvdFields_r10b, GrpAdminDeleteIOCQCmd)
        APPEND_TEST_AT_XLEVEL(InvalidQID_r10b, GrpAdminDeleteIOCQCmd)
        APPEND_TEST_AT_XLEVEL(DeleteAllAtOnce_r10b, GrpAdminDeleteIOCQCmd)
        APPEND_TEST_AT_XLEVEL(DeleteFullQ_r10b, GrpAdminDeleteIOCQCmd)
        break;

    default:
    case SPECREVTYPE_FENCE:
        throw FrmwkEx(HERE, "Object created with an unknown SpecRev=%d",
            gCmdLine.rev);
    }
}


GrpAdminDeleteIOCQCmd::~GrpAdminDeleteIOCQCmd()
{
    // mTests deallocated in parent
}

}   // namespace
