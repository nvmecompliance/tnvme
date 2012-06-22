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

#include "grpAdminCreateIOQCmd.h"
#include "../Exception/frmwkEx.h"
#include "prpLessPageContig_r10b.h"
#include "prpLessPageDiscontig_r10b.h"
#include "prpSinglePageContig_r10b.h"
#include "prpSinglePageDiscontig_r10b.h"
#include "prpGreaterPageContig_r10b.h"
#include "prpGreaterPageDiscontig_r10b.h"

namespace GrpAdminCreateIOQCmd {


GrpAdminCreateIOQCmd::GrpAdminCreateIOQCmd(size_t grpNum) :
    Group(grpNum, "GrpAdminCreateIOQCmd",
        "Admin cmd set common create IOCQ/IOSQ test cases")
{
    // For complete details about the APPEND_TEST_AT_?LEVEL() macros:
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering" and
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Strategy
    switch (gCmdLine.rev) {
    case SPECREV_10b:
        APPEND_TEST_AT_XLEVEL(PRPLessPageContig_r10b, GrpAdminCreateIOQCmd)
        APPEND_TEST_AT_XLEVEL(PRPLessPageDiscontig_r10b, GrpAdminCreateIOQCmd)
        APPEND_TEST_AT_XLEVEL(PRPSinglePageContig_r10b, GrpAdminCreateIOQCmd)
        APPEND_TEST_AT_XLEVEL(PRPSinglePageDiscontig_r10b, GrpAdminCreateIOQCmd)
        APPEND_TEST_AT_XLEVEL(PRPGreaterPageContig_r10b, GrpAdminCreateIOQCmd)
        APPEND_TEST_AT_XLEVEL(PRPGreaterPageDiscontig_r10b, GrpAdminCreateIOQCmd)
        break;

    default:
    case SPECREVTYPE_FENCE:
        throw FrmwkEx(HERE, "Object created with an unknown SpecRev=%d",
            gCmdLine.rev);
    }
}


GrpAdminCreateIOQCmd::~GrpAdminCreateIOQCmd()
{
    // mTests deallocated in parent
}

}   // namespace
