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


GrpBasicInit::GrpBasicInit(size_t grpNum, SpecRev specRev, ErrorRegs errRegs,
    int fd) :
    Group(grpNum, specRev, "GrpBasicInit", "Basic Initialization")
{
    // ------------------------CHANGE NOTICE: (3-2-2012)------------------------
    // The rule to keep groups and tests at a well known constant reference
    // number for all of time is to restrictive. A new scheme has replaced
    // that strategy. For complete details refer to:
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering" and
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Strategy"
    switch (mSpecRev) {
    case SPECREV_10b:
        APPEND_TEST_AT_XLEVEL(CreateACQASQ_r10b, fd, GrpBasicInit, errRegs)
        // Polling 1st
        APPEND_TEST_AT_ZLEVEL(CreateIOQContigPoll_r10b, fd, GrpBasicInit, errRegs)
        APPEND_TEST_AT_ZLEVEL(CreateIOQDiscontigPoll_r10b, fd, GrpBasicInit, errRegs)
        APPEND_TEST_AT_ZLEVEL(WriteDataPat_r10b, fd, GrpBasicInit, errRegs)
        APPEND_TEST_AT_ZLEVEL(VerifyDataPat_r10b, fd, GrpBasicInit, errRegs)
        APPEND_TEST_AT_ZLEVEL(DeleteIOQContig_r10b, fd, GrpBasicInit, errRegs)
        APPEND_TEST_AT_ZLEVEL(DeleteIOQDiscontig_r10b, fd, GrpBasicInit, errRegs)
        // IRQ 2nd
        APPEND_TEST_AT_ZLEVEL(CreateIOQContigIrq_r10b, fd, GrpBasicInit, errRegs)
        APPEND_TEST_AT_ZLEVEL(CreateIOQDiscontigIrq_r10b, fd, GrpBasicInit, errRegs)
        APPEND_TEST_AT_ZLEVEL(WriteDataPat_r10b, fd, GrpBasicInit, errRegs)
        APPEND_TEST_AT_ZLEVEL(VerifyDataPat_r10b, fd, GrpBasicInit, errRegs)
        APPEND_TEST_AT_ZLEVEL(DeleteIOQContig_r10b, fd, GrpBasicInit, errRegs)
        APPEND_TEST_AT_ZLEVEL(DeleteIOQDiscontig_r10b, fd, GrpBasicInit, errRegs)
        break;

    default:
    case SPECREVTYPE_FENCE:
        throw FrmwkEx(HERE, "Object created with an unknown SpecRev=%d", specRev);
    }
}


GrpBasicInit::~GrpBasicInit()
{
    // mTests deallocated in parent
}

}   // namespace
