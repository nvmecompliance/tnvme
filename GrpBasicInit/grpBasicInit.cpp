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

#include "grpBasicInit.h"
#include "createACQASQ_r10b.h"
#include "createIOQContigPoll_r10b.h"
#include "createIOQDiscontigPoll_r10b.h"
#include "writeDataPat_r10b.h"
#include "verifyDataPat_r10b.h"
#include "deleteIOQContig_r10b.h"
#include "deleteIOQDiscontig_r10b.h"
#include "createIOQContigIrq_r10b.h"
#include "createIOQDiscontigIrq_r10b.h"

namespace GrpBasicInit {


GrpBasicInit::GrpBasicInit(size_t grpNum) :
    Group(grpNum, "GrpBasicInit", "Basic Initialization")
{
    // For complete details about the APPEND_TEST_AT_?LEVEL() macros:
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering" and
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Strategy
    switch (gCmdLine.rev) {
    case SPECREV_10b:
        APPEND_TEST_AT_XLEVEL(CreateACQASQ_r10b, GrpBasicInit)
        // Polling 1st
        APPEND_TEST_AT_ZLEVEL(CreateIOQContigPoll_r10b, GrpBasicInit)
        APPEND_TEST_AT_ZLEVEL(CreateIOQDiscontigPoll_r10b, GrpBasicInit)
        APPEND_TEST_AT_ZLEVEL(WriteDataPat_r10b, GrpBasicInit)
        APPEND_TEST_AT_ZLEVEL(VerifyDataPat_r10b, GrpBasicInit)
        APPEND_TEST_AT_ZLEVEL(DeleteIOQContig_r10b, GrpBasicInit)
        APPEND_TEST_AT_ZLEVEL(DeleteIOQDiscontig_r10b, GrpBasicInit)
        // IRQ 2nd
        APPEND_TEST_AT_ZLEVEL(CreateIOQContigIrq_r10b, GrpBasicInit)
        APPEND_TEST_AT_ZLEVEL(CreateIOQDiscontigIrq_r10b, GrpBasicInit)
        APPEND_TEST_AT_ZLEVEL(WriteDataPat_r10b, GrpBasicInit)
        APPEND_TEST_AT_ZLEVEL(VerifyDataPat_r10b, GrpBasicInit)
        APPEND_TEST_AT_ZLEVEL(DeleteIOQContig_r10b, GrpBasicInit)
        APPEND_TEST_AT_ZLEVEL(DeleteIOQDiscontig_r10b, GrpBasicInit)
        break;

    default:
    case SPECREVTYPE_FENCE:
        throw FrmwkEx(HERE, "Object created with an unknown SpecRev=%d",
            gCmdLine.rev);
    }
}


GrpBasicInit::~GrpBasicInit()
{
    // mTests deallocated in parent
}

}   // namespace
