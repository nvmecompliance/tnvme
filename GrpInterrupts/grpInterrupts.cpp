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

#include "grpInterrupts.h"
#include "../Exception/frmwkEx.h"
#include "createResources_r10b.h"
#include "invalidMSIXIRQ_r10b.h"
#include "partialReapMSIX_r10b.h"
#include "maxIOQMSIX1To1_r10b.h"
#include "maxIOQMSIXManyTo1_r10b.h"

namespace GrpInterrupts {


GrpInterrupts::GrpInterrupts(size_t grpNum) :
    Group(grpNum, "GrpInterrupts", "MSI-single, MSI-multi, MSI-X test cases")
{
    // For complete details about the APPEND_TEST_AT_?LEVEL() macros:
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering" and
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Strategy
    switch (gCmdLine.rev) {
    case SPECREV_10b:
        APPEND_TEST_AT_XLEVEL(CreateResources_r10b, GrpInterrupts)
        APPEND_TEST_AT_YLEVEL(InvalidMSIXIRQ_r10b, GrpInterrupts)
        APPEND_TEST_AT_YLEVEL(PartialReapMSIX_r10b, GrpInterrupts)
        APPEND_TEST_AT_YLEVEL(MaxIOQMSIX1To1_r10b, GrpInterrupts)
        APPEND_TEST_AT_YLEVEL(MaxIOQMSIXManyTo1_r10b, GrpInterrupts)
        break;

    default:
    case SPECREVTYPE_FENCE:
        throw FrmwkEx(HERE, "Object created with an unknown SpecRev=%d",
            gCmdLine.rev);
    }
}


GrpInterrupts::~GrpInterrupts()
{
    // mTests deallocated in parent
}

}   // namespace
