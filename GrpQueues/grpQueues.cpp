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
#include "adminQFull_r10b.h"
#include "ioqFull_r10b.h"
#include "manyCmdSubmit_r10b.h"
#include "illegalDeleteQs_r10b.h"
#include "illegalCreateOrder_r10b.h"
#include "maxIOQ_r10b.h"
#include "sqcqSizeMismatch_r10b.h"
#include "qIdVariations_r10b.h"
#include "illegalCreateQs_r10b.h"

namespace GrpQueues {


GrpQueues::GrpQueues(size_t grpNum) :
    Group(grpNum, "GrpQueues", "Validates general queue functionality")
{
    // For complete details about the APPEND_TEST_AT_?LEVEL() macros:
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering" and
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Strategy
    switch (gCmdLine.rev) {
    case SPECREV_10b:
        APPEND_TEST_AT_XLEVEL(InitialStateAdmin_r10b, GrpQueues)
        APPEND_TEST_AT_XLEVEL(AdminQRollChkSame_r10b, GrpQueues)
        APPEND_TEST_AT_XLEVEL(AdminQRollChkDiff_r10b, GrpQueues)
        APPEND_TEST_AT_XLEVEL(AdminQFull_r10b, GrpQueues)
        APPEND_TEST_AT_XLEVEL(IllegalCreateOrder_r10b, GrpQueues)
        APPEND_TEST_AT_XLEVEL(CreateResources_r10b, GrpQueues)
        APPEND_TEST_AT_YLEVEL(IOQRollChkSame_r10b, GrpQueues)
        APPEND_TEST_AT_YLEVEL(IOQRollChkDiff_r10b, GrpQueues)
        APPEND_TEST_AT_YLEVEL(ManySQtoCQAssoc_r10b, GrpQueues)
        APPEND_TEST_AT_YLEVEL(IOQFull_r10b, GrpQueues)
        APPEND_TEST_AT_YLEVEL(ManyCmdSubmit_r10b, GrpQueues)
        APPEND_TEST_AT_YLEVEL(IllegalDeleteQs_r10b, GrpQueues)
        APPEND_TEST_AT_YLEVEL(MaxIOQ_r10b, GrpQueues)
        APPEND_TEST_AT_YLEVEL(SQCQSizeMismatch_r10b, GrpQueues)
        APPEND_TEST_AT_YLEVEL(QIDVariations_r10b, GrpQueues)
        APPEND_TEST_AT_YLEVEL(IllegalCreateQs_r10b, GrpQueues)

        break;

    default:
    case SPECREVTYPE_FENCE:
        throw FrmwkEx(HERE, "Object created with an unknown SpecRev=%d",
            gCmdLine.rev);
    }
}


GrpQueues::~GrpQueues()
{
    // mTests deallocated in parent
}

}   // namespace
