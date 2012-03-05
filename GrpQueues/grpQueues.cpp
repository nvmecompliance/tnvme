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

#include "grpQueues.h"
#include "initialStateAdmin_r10b.h"
#include "adminQRollChkSame_r10b.h"
#include "adminQRollChkDiff_r10b.h"
#include "createResources_r10b.h"
#include "ioqRollChkSame_r10b.h"
#include "ioqRollChkDiff_r10b.h"
#include "manySQtoCQAssoc_r10b.h"

namespace GrpQueues {


GrpQueues::GrpQueues(size_t grpNum, SpecRev specRev, ErrorRegs errRegs,
    int fd) :
    Group(grpNum, specRev, "Validates general queue functionality")
{
    // ------------------------CHANGE NOTICE: (3-2-2012)------------------------
    // The rule to keep groups and tests at a well known constant reference
    // number for all of time is to restrictive. A new scheme has replaced
    // that strategy. For complete details refer to:
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering" and
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Strategy
    switch (mSpecRev) {
    case SPECREV_10b:
        APPEND_TEST_AT_XLEVEL(InitialStateAdmin_r10b, fd, GrpQueues, errRegs)
        APPEND_TEST_AT_XLEVEL(AdminQRollChkSame_r10b, fd, GrpQueues, errRegs)
        APPEND_TEST_AT_XLEVEL(AdminQRollChkDiff_r10b, fd, GrpQueues, errRegs)
        APPEND_TEST_AT_XLEVEL(CreateResources_r10b, fd, GrpQueues, errRegs)
        APPEND_TEST_AT_YLEVEL(IOQRollChkSame_r10b, fd, GrpQueues, errRegs)
        APPEND_TEST_AT_YLEVEL(IOQRollChkDiff_r10b, fd, GrpQueues, errRegs)
        APPEND_TEST_AT_YLEVEL(ManySQtoCQAssoc_r10b, fd, GrpQueues, errRegs)
        break;

    default:
    case SPECREVTYPE_FENCE:
        LOG_DBG("Object created with an unknown SpecRev=%d", specRev);
        break;
    }
}


GrpQueues::~GrpQueues()
{
    // mTests deallocated in parent
}

}   // namespace
