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

#include "grpAdminDeleteIOSQCmd.h"
#include "../Exception/frmwkEx.h"
#include "createResources_r10b.h"
#include "unsupportRsvdFields_r10b.h"
#include "invalidQID_r10b.h"
#include "deleteAllAtOnce_r10b.h"

namespace GrpAdminDeleteIOSQCmd {


GrpAdminDeleteIOSQCmd::GrpAdminDeleteIOSQCmd(size_t grpNum, SpecRev specRev, ErrorRegs errRegs,
    int fd) :
    Group(grpNum, specRev, "GrpAdminDeleteIOSQCmd",
        "Admin cmd set delete IOSQ test cases")
{
    // For complete details about the APPEND_TEST_AT_?LEVEL() macros:
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering" and
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Strategy
    switch (mSpecRev) {
    case SPECREV_10b:
        APPEND_TEST_AT_XLEVEL(CreateResources_r10b, fd, GrpAdminDeleteIOSQCmd, errRegs)
        APPEND_TEST_AT_YLEVEL(UnsupportRsvdFields_r10b, fd, GrpAdminDeleteIOSQCmd, errRegs)
        APPEND_TEST_AT_XLEVEL(InvalidQID_r10b, fd, GrpAdminDeleteIOSQCmd, errRegs)
        APPEND_TEST_AT_XLEVEL(DeleteAllAtOnce_r10b, fd, GrpAdminDeleteIOSQCmd, errRegs)
        break;

    default:
    case SPECREVTYPE_FENCE:
        throw FrmwkEx(HERE, "Object created with an unknown SpecRev=%d", specRev);
    }
}


GrpAdminDeleteIOSQCmd::~GrpAdminDeleteIOSQCmd()
{
    // mTests deallocated in parent
}

}   // namespace
