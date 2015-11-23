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

#include "grpNVMWriteCmd.h"
#include "createResources_r10b.h"
#include "lbaOutOfRangeBare_r10b.h"
#include "lbaOutOfRangeBare_r12.h"
#include "invalidNamspc_r10b.h"
#include "unsupportRsvdFields_r10b.h"
#include "unsupportRsvdFields_r11b.h"
#include "unsupportRsvdFields_r12.h"
#include "ignoreMetaPtrBare_r10b.h"
#include "ignoreMetaPtrBare_r12.h"
#include "protInfoIgnoreBare_r10b.h"
#include "protInfoIgnoreBare_r12.h"
#include "FUA_r10b.h"
#include "limitedRetry_r10b.h"
#include "lbaOutOfRangeMeta_r10b.h"
#include "lbaOutOfRangeMeta_r12.h"
#include "ignoreMetaPtrMeta_r10b.h"
#include "ignoreMetaPtrMeta_r12.h"
#include "protInfoIgnoreMeta_r10b.h"
#include "protInfoIgnoreMeta_r12.h"

namespace GrpNVMWriteCmd {


GrpNVMWriteCmd::GrpNVMWriteCmd(size_t grpNum) :
    Group(grpNum, "GrpNVMWriteCmd", "NVM cmd set write cmd tests")
{
    // For complete details about the APPEND_TEST_AT_?LEVEL() macros:
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering" and
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Strategy
    switch (gCmdLine.rev) {
    case SPECREV_10b:
        APPEND_TEST_AT_XLEVEL(CreateResources_r10b, GrpNVMWriteCmd)
        APPEND_TEST_AT_YLEVEL(LBAOutOfRangeBare_r10b, GrpNVMWriteCmd)
        APPEND_TEST_AT_YLEVEL(InvalidNamspc_r10b, GrpNVMWriteCmd)
        APPEND_TEST_AT_YLEVEL(UnsupportRsvdFields_r10b, GrpNVMWriteCmd)
        APPEND_TEST_AT_YLEVEL(IgnoreMetaPtrBare_r10b, GrpNVMWriteCmd)
        APPEND_TEST_AT_YLEVEL(ProtInfoIgnoreBare_r10b, GrpNVMWriteCmd)
        APPEND_TEST_AT_YLEVEL(FUA_r10b, GrpNVMWriteCmd)
        APPEND_TEST_AT_YLEVEL(LimitedRetry_r10b, GrpNVMWriteCmd)
        APPEND_TEST_AT_YLEVEL(IgnoreMetaPtrMeta_r10b, GrpNVMWriteCmd)
        APPEND_TEST_AT_XLEVEL(LBAOutOfRangeMeta_r10b, GrpNVMWriteCmd)
        APPEND_TEST_AT_XLEVEL(ProtInfoIgnoreMeta_r10b, GrpNVMWriteCmd)
        break;
    case SPECREV_11:
        APPEND_TEST_AT_XLEVEL(CreateResources_r10b, GrpNVMWriteCmd)
        APPEND_TEST_AT_YLEVEL(LBAOutOfRangeBare_r10b, GrpNVMWriteCmd)
        APPEND_TEST_AT_YLEVEL(InvalidNamspc_r10b, GrpNVMWriteCmd)
        APPEND_TEST_AT_YLEVEL(UnsupportRsvdFields_r11b, GrpNVMWriteCmd)
        APPEND_TEST_AT_YLEVEL(IgnoreMetaPtrBare_r10b, GrpNVMWriteCmd)
        APPEND_TEST_AT_YLEVEL(ProtInfoIgnoreBare_r10b, GrpNVMWriteCmd)
        APPEND_TEST_AT_YLEVEL(FUA_r10b, GrpNVMWriteCmd)
        APPEND_TEST_AT_YLEVEL(LimitedRetry_r10b, GrpNVMWriteCmd)
        APPEND_TEST_AT_YLEVEL(IgnoreMetaPtrMeta_r10b, GrpNVMWriteCmd)
        APPEND_TEST_AT_XLEVEL(LBAOutOfRangeMeta_r10b, GrpNVMWriteCmd)
        APPEND_TEST_AT_XLEVEL(ProtInfoIgnoreMeta_r10b, GrpNVMWriteCmd)
        break;
    case SPECREV_12:
        APPEND_TEST_AT_XLEVEL(CreateResources_r10b, GrpNVMReadCmd)
        APPEND_TEST_AT_YLEVEL(LBAOutOfRangeBare_r12, GrpNVMReadCmd)
        APPEND_TEST_AT_YLEVEL(InvalidNamspc_r10b, GrpNVMReadCmd)
        APPEND_TEST_AT_YLEVEL(UnsupportRsvdFields_r12, GrpNVMReadCmd)
        APPEND_TEST_AT_YLEVEL(ProtInfoIgnoreBare_r12, GrpNVMReadCmd)
        APPEND_TEST_AT_YLEVEL(IgnoreMetaPtrBare_r12, GrpNVMReadCmd)
        APPEND_TEST_AT_YLEVEL(FUA_r10b, GrpNVMReadCmd)
        APPEND_TEST_AT_YLEVEL(LimitedRetry_r10b, GrpNVMReadCmd)
        APPEND_TEST_AT_YLEVEL(IgnoreMetaPtrMeta_r12, GrpNVMReadCmd)
        APPEND_TEST_AT_XLEVEL(LBAOutOfRangeMeta_r12, GrpNVMReadCmd)
        APPEND_TEST_AT_XLEVEL(ProtInfoIgnoreMeta_r12, GrpNVMReadCmd)
        break;
    default:
    case SPECREVTYPE_FENCE:
        throw FrmwkEx(HERE, "Object created with an unknown SpecRev=%d",
            gCmdLine.rev);
    }
}


GrpNVMWriteCmd::~GrpNVMWriteCmd()
{
    // mTests deallocated in parent
}

}   // namespace
