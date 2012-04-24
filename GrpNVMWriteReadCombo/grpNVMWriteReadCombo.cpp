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

#include "grpNVMWriteReadCombo.h"
#include "createResources_r10b.h"
#include "prpOffsetSinglePgSingleBlk_r10b.h"
#include "prpOffsetSinglePgMultiBlk_r10b.h"
#include "prpOffsetDualPgMultiBlk_r10b.h"
#include "prpOffsetMultiPgMultiBlk_r10b.h"
#include "startingLBABare_r10b.h"
#include "nlbaBare_r10b.h"
#include "datasetMgmt_r10b.h"

namespace GrpNVMWriteReadCombo {


GrpNVMWriteReadCombo::GrpNVMWriteReadCombo(size_t grpNum, SpecRev specRev,
    ErrorRegs errRegs, int fd) :
    Group(grpNum, specRev, "NVM cmd set write/read combo tests.")
{
    // ------------------------CHANGE NOTICE: (3-2-2012)------------------------
    // The rule to keep groups and tests at a well known constant reference
    // number for all of time is too restrictive. A new scheme has replaced
    // that strategy. For complete details refer to:
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering" and
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Strategy
    switch (mSpecRev) {
    case SPECREV_10b:
        APPEND_TEST_AT_XLEVEL(PRPOffsetSinglePgMultiBlk_r10b, fd, GrpNVMWriteReadCombo, errRegs)
        APPEND_TEST_AT_XLEVEL(PRPOffsetDualPgMultiBlk_r10b, fd, GrpNVMWriteReadCombo, errRegs)
        APPEND_TEST_AT_XLEVEL(PRPOffsetMultiPgMultiBlk_r10b, fd, GrpNVMWriteReadCombo, errRegs)
        APPEND_TEST_AT_XLEVEL(CreateResources_r10b, fd, GrpNVMWriteReadCombo, errRegs)
        APPEND_TEST_AT_YLEVEL(PRPOffsetSinglePgSingleBlk_r10b, fd, GrpNVMWriteReadCombo, errRegs)
        APPEND_TEST_AT_YLEVEL(StartingLBABare_r10b, fd, GrpNVMWriteReadCombo, errRegs)
        APPEND_TEST_AT_YLEVEL(NLBABare_r10b, fd, GrpNVMWriteReadCombo, errRegs)
        APPEND_TEST_AT_YLEVEL(DatasetMgmt_r10b, fd, GrpNVMWriteReadCombo, errRegs)

        break;

    default:
    case SPECREVTYPE_FENCE:
        throw FrmwkEx(HERE, "Object created with an unknown SpecRev=%d", specRev);
    }
}


GrpNVMWriteReadCombo::~GrpNVMWriteReadCombo()
{
    // mTests deallocated in parent
}

}   // namespace
