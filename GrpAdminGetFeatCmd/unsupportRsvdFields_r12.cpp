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

#include <boost/utility/binary.hpp>

#include "unsupportRsvdFields_r12.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/io.h"
#include "../Cmds/getFeatures.h"

#define FEATURE_ID      0x01

namespace GrpAdminGetFeatCmd {


UnsupportRsvdFields_r12::UnsupportRsvdFields_r12(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_12)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.2, section 5.9");
    mTestDesc.SetShort(     "Set unsupported/rsvd fields in cmd");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Unsupported DW's and rsvd fields are treated identical, the recipient "
        "is not required to check their value. Receipt of reserved coded "
        "values shall be reported as an error. Issue a GetFeature cmd with "
        "DW10.FID = 0x01, and set bits DW0_b14:10, DW2, DW3, DW4, DW5, "
        "DW10_31:11, DW12, DW13, DW14, DW15, expect success. Issue same cmd "
        "setting all rsvd coded values, expect fail. Set: DW10_b10:8, "
        "DW10_b7:0");
}


UnsupportRsvdFields_r12::~UnsupportRsvdFields_r12()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


UnsupportRsvdFields_r12::
UnsupportRsvdFields_r12(const UnsupportRsvdFields_r12 &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


UnsupportRsvdFields_r12 &
UnsupportRsvdFields_r12::operator=(const UnsupportRsvdFields_r12 &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
UnsupportRsvdFields_r12::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////
    if (gCmdLine.rsvdfields == false)
        return RUN_FALSE;   // Optional rsvd fields test skipped.

    preserve = preserve;    // Suppress compiler error/warning
    return RUN_TRUE;        // This test is never destructive
}


void
UnsupportRsvdFields_r12::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    LOG_NRM("Create Get features cmd");
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());
    getFeaturesCmd->SetFID(FEATURE_ID);

    LOG_NRM("Set all cmd's rsvd bits");
    uint32_t work = getFeaturesCmd->GetDword(0);
    work |= 0x00003c00;      // Set DW0_b13:10 bits
    getFeaturesCmd->SetDword(work, 0);

    getFeaturesCmd->SetDword(0xffffffff, 2);
    getFeaturesCmd->SetDword(0xffffffff, 3);
    getFeaturesCmd->SetDword(0xffffffff, 4);
    getFeaturesCmd->SetDword(0xffffffff, 5);
    getFeaturesCmd->SetDword(0xffffffff, 6);
    getFeaturesCmd->SetDword(0xffffffff, 7);
    getFeaturesCmd->SetDword(0xffffffff, 8);
    getFeaturesCmd->SetDword(0xffffffff, 9);

    // DW10_b31:11
    work = getFeaturesCmd->GetDword(10);
    work |= 0xffffc000;
    getFeaturesCmd->SetDword(work, 10);

    getFeaturesCmd->SetDword(0xffffffff, 12);
    getFeaturesCmd->SetDword(0xffffffff, 13);
    getFeaturesCmd->SetDword(0xffffffff, 14);
    getFeaturesCmd->SetDword(0xffffffff, 15);

    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
        getFeaturesCmd, "rsvd.set", true);

    uint32_t cdw10 = getFeaturesCmd->GetDword(10) & ~0x3ff;
    uint64_t oncs = gInformative->GetIdentifyCmdCtrlr()->GetValue(
            IDCTRLRCAP_ONCS);
    uint64_t savSelSupp = (oncs & ONCS_SUP_SV_AND_SLCT_FEATS) >> 4;

    if (savSelSupp != 0) {
        LOG_NRM("Select field supported, test reserved values");
        for (uint32_t select = BOOST_BINARY(100); select <= BOOST_BINARY(111);
                ++select) {
            work = cdw10 | (select << 8);
            getFeaturesCmd->SetDword(work, 10);

            IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq,
                    acq, getFeaturesCmd, "rsvd.val.set", true,
                    CESTAT_INVAL_FIELD);
        }
    }

    LOG_NRM("Test reserved FID values");
    for (uint32_t fid = 0xd; fid <= 0x7f; ++fid) {
        work = cdw10 | fid;
        getFeaturesCmd->SetDword(work, 10);

        IO::SendAndReapCmdNot(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
                getFeaturesCmd, "rsvd.val.set", true, CESTAT_SUCCESS);
    }
}

}   // namespace
