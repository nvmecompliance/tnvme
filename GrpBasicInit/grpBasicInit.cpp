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
    Group(grpNum, specRev, "Basic Initialization")
{
    // IMPORTANT: Once a test case is assigned a position in the vector, i.e.
    //            a test index/number of the form major.minor, then it should
    //            reside in that position forever so test reference numbers
    //            don't change per release. Future tests can be appended at
    //            either the group level or the test level.
    //            Tests 0.0, 1.0, <next_test_num=2>.0  Major num; group level
    //            Tests x.0, x.1, x.<next_test_num=2>  Minor num; test level
    switch (mSpecRev) {
    case SPECREV_10b:
        APPEND_TEST_AT_GROUP_LEVEL(CreateACQASQ_r10b, fd, GrpBasicInit, errRegs)
        APPEND_TEST_AT_GROUP_LEVEL(CreateIOQContigPoll_r10b, fd, GrpBasicInit, errRegs)
        APPEND_TEST_AT_GROUP_LEVEL(CreateIOQDiscontigPoll_r10b, fd, GrpBasicInit, errRegs)
        APPEND_TEST_AT_GROUP_LEVEL(WriteDataPat_r10b, fd, GrpBasicInit, errRegs)
        APPEND_TEST_AT_GROUP_LEVEL(VerifyDataPat_r10b, fd, GrpBasicInit, errRegs)
        APPEND_TEST_AT_GROUP_LEVEL(DeleteIOQContig_r10b, fd, GrpBasicInit, errRegs)
        APPEND_TEST_AT_GROUP_LEVEL(DeleteIOQDiscontig_r10b, fd, GrpBasicInit, errRegs)

        APPEND_TEST_AT_GROUP_LEVEL(CreateIOQContigIrq_r10b, fd, GrpBasicInit, errRegs)
        APPEND_TEST_AT_GROUP_LEVEL(CreateIOQDiscontigIrq_r10b, fd, GrpBasicInit, errRegs)
        APPEND_TEST_AT_GROUP_LEVEL(WriteDataPat_r10b, fd, GrpBasicInit, errRegs)
        APPEND_TEST_AT_GROUP_LEVEL(VerifyDataPat_r10b, fd, GrpBasicInit, errRegs)
        APPEND_TEST_AT_GROUP_LEVEL(DeleteIOQContig_r10b, fd, GrpBasicInit, errRegs)
        APPEND_TEST_AT_GROUP_LEVEL(DeleteIOQDiscontig_r10b, fd, GrpBasicInit, errRegs)
        break;

    default:
    case SPECREVTYPE_FENCE:
        LOG_DBG("Object created with an unknown SpecRev=%d", specRev);
        break;
    }
}


GrpBasicInit::~GrpBasicInit()
{
    // mTests deallocated in parent
}

}   // namespace
