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

#include "unsupportRrvdFields_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/io.h"
#include "../Cmds/getLogPage.h"

#define FIRM_SLOT_INFO_LID      0x03
#define PRP1_ONLY_NUMD          (514 / 4)
#define BUFFER_OFFSET           0

namespace GrpAdminGetLogPgCmd {


UnsupportRrvdFields_r10b::UnsupportRrvdFields_r10b(string grpName,
    string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Set unsupported/rsvd fields in cmd");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Unsupported DW's and rsvd fields are treated identical, the "
        "recipient shall not check their value. Issue GetLogPage cmd, "
        "LID=3, NUMD=(512/4), expect success. Then issue same cmd setting "
        "all unsupported/rsvd fields, expect success. Set: DW0_b15:10, "
        "DW2, DW3, DW4, DW5, DW10_b31:28, DW11, DW12, DW13, DW14, DW15.");
}


UnsupportRrvdFields_r10b::~UnsupportRrvdFields_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


UnsupportRrvdFields_r10b::
UnsupportRrvdFields_r10b(const UnsupportRrvdFields_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


UnsupportRrvdFields_r10b &
UnsupportRrvdFields_r10b::operator=(const UnsupportRrvdFields_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
UnsupportRrvdFields_r10b::RunnableCoreTest(bool preserve)
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
UnsupportRrvdFields_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     *  \endverbatim
     */
    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    LOG_NRM("Create get log page cmd and assoc some buffer memory");
    SharedGetLogPagePtr getLogPgCmd = SharedGetLogPagePtr(new GetLogPage());

    LOG_NRM("Get log page to request firmware slot information");
    getLogPgCmd->SetNUMD(PRP1_ONLY_NUMD - 1); // 0-based
    getLogPgCmd->SetLID(FIRM_SLOT_INFO_LID);

    SharedMemBufferPtr getLogPageMem = SharedMemBufferPtr(new MemBuffer());
    getLogPageMem->InitOffset1stPage(GetLogPage::FIRMSLOT_DATA_SIZE,
        BUFFER_OFFSET, true);
    send_64b_bitmask prpReq =
        (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    getLogPgCmd->SetPrpBuffer(prpReq, getLogPageMem);

    LOG_NRM("Issue GetLogPage cmd without setting reserved bits.");
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
        getLogPgCmd, "rsvd.notset", true);

    LOG_NRM("Set all cmd's rsvd bits");
    uint32_t work = getLogPgCmd->GetDword(0);
    work |= 0x0000fc00;      // Set DW0_b15:10 bits
    getLogPgCmd->SetDword(work, 0);

    getLogPgCmd->SetDword(0xffffffff, 2);
    getLogPgCmd->SetDword(0xffffffff, 3);
    getLogPgCmd->SetDword(0xffffffff, 4);
    getLogPgCmd->SetDword(0xffffffff, 5);

    work = getLogPgCmd->GetDword(10);
    work |= 0xf0000000;      // Set DW10_b31:28 bits
    getLogPgCmd->SetDword(work, 10);

    getLogPgCmd->SetDword(0xffffffff, 11);
    getLogPgCmd->SetDword(0xffffffff, 12);
    getLogPgCmd->SetDword(0xffffffff, 13);
    getLogPgCmd->SetDword(0xffffffff, 14);
    getLogPgCmd->SetDword(0xffffffff, 15);

    LOG_NRM("Issue GetLogPage cmd after setting reserved bits.");
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
        getLogPgCmd, "rsvd.set", true);

}

}   // namespace
