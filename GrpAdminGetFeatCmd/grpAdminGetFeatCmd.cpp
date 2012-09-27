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

#include "grpAdminGetFeatCmd.h"
#include "createResources_r10b.h"
#include "invalidFieldInCmd_r10b.h"
#include "unsupportRrvdFields_r10b.h"


namespace GrpAdminGetFeatCmd {


GrpAdminGetFeatCmd::GrpAdminGetFeatCmd(size_t grpNum) :
    Group(grpNum, "GrpAdminGetFeatCmd",
        "Admin cmd set features test cases w/o get features")
{
    // For complete details about the APPEND_TEST_AT_?LEVEL() macros:
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering" and
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Strategy
    switch (gCmdLine.rev) {
    case SPECREV_10b:
        APPEND_TEST_AT_XLEVEL(CreateResources_r10b, GrpAdminGetFeatCmd)
        APPEND_TEST_AT_YLEVEL(InvalidFieldInCmd_r10b, GrpAdminGetFeatCmd)
        APPEND_TEST_AT_YLEVEL(UnsupportRrvdFields_r10b, GrpAdminGetFeatCmd)
        break;

    default:
    case SPECREVTYPE_FENCE:
        throw FrmwkEx(HERE, "Object created with an unknown SpecRev=%d",
            gCmdLine.rev);
    }
}


GrpAdminGetFeatCmd::~GrpAdminGetFeatCmd()
{
    // mTests deallocated in parent
}

}   // namespace
