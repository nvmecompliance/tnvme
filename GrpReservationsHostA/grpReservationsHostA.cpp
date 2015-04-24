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

#include "grpReservationsHostA.h"
#include "createResources_r11.h"
#include "registerReservation.h"
#include "replaceReservation.h"
#include "acquireReservation.h"
#include "releaseReservation.h"

namespace GrpReservationsHostA {

GrpReservationsHostA::GrpReservationsHostA(size_t grpNum) :
		Group(grpNum, "GrpReservationsHostA", "Reservation Commands HostA")
{
    // For complete details about the APPEND_TEST_AT_?LEVEL() macros:
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering" and
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Strategy
    switch (gCmdLine.rev) {
    case SPECREV_10b:
        break;
    case SPECREV_12:
    case SPECREV_11:
        APPEND_TEST_AT_XLEVEL(CreateResources_r11, GrpReservationsHostA)
        APPEND_TEST_AT_YLEVEL(RegisterReservation, GrpReservationsHostA)
        APPEND_TEST_AT_YLEVEL(ReplaceReservation, GrpReservationsHostA)
        APPEND_TEST_AT_YLEVEL(AcquireReservation, GrpReservationsHostA)
        APPEND_TEST_AT_YLEVEL(ReleaseReservation, GrpReservationsHostA)
        break;

    default:
    case SPECREVTYPE_FENCE:
        throw FrmwkEx(HERE, "Object created with an unknown SpecRev=%d",
            gCmdLine.rev);
    }
}

GrpReservationsHostA::~GrpReservationsHostA()
{
    // mTests deallocated in parent
}

}   // namespace
