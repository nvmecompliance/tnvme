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

#include <boost/format.hpp>

#include "unsupportRsvdFields_r11b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/io.h"
#include "../Cmds/getLogPage.h"

#define FIRM_SLOT_INFO_LID      0x03
#define PRP1_ONLY_NUMD          (514 / 4)
#define BUFFER_OFFSET           0

namespace GrpAdminGetLogPgCmd {


UnsupportRsvdFields_r11b::UnsupportRsvdFields_r11b(string grpName,
    string testName) :
    Test(grpName, testName, SPECREV_11)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.1b, section 5.10");
    mTestDesc.SetShort(     "Set unsupported/rsvd fields in cmd");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Unsupported DW's and rsvd fields are treated identical, the "
        "recipient is not required to check their value. Receipt of reserved "
        "coded values shall be reported as an error. Issue GetLogPage cmd, "
        "LID=3, NUMD=(512/4), expect success. Then issue same cmd setting "
        "all unsupported/rsvd fields, expect success. Set: DW0_b14:10, "
        "DW2, DW3, DW4, DW5, DW10_b31:28, DW11, DW12, DW13, DW14, DW15. Issue "
        "same cmd setting all rsvd coded values, expect fail.  Set: DW10_b7:0");
}


UnsupportRsvdFields_r11b::~UnsupportRsvdFields_r11b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


UnsupportRsvdFields_r11b::
UnsupportRsvdFields_r11b(const UnsupportRsvdFields_r11b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


UnsupportRsvdFields_r11b &
UnsupportRsvdFields_r11b::operator=(const UnsupportRsvdFields_r11b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
UnsupportRsvdFields_r11b::RunnableCoreTest(bool preserve)
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
UnsupportRsvdFields_r11b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     *  \endverbatim
     */
    // Lookup objs which were created in a prior test within group
    string globalWork;
    //uint64_t i;
    
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    LOG_NRM("Create get log page cmd and assoc some buffer memory");
    SharedGetLogPagePtr getLogPgCmd = SharedGetLogPagePtr(new GetLogPage());

    LOG_NRM("Get log page to request firmware slot information");
    getLogPgCmd->SetNUMD(PRP1_ONLY_NUMD - 1); // 0-based
    getLogPgCmd->SetLID(FIRM_SLOT_INFO_LID);
    getLogPgCmd->SetNSID(0xFFFFFFFF);
    
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
    work |= 0x00007c00;      // Set DW0_b14:10 bits
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

    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
        getLogPgCmd, "rsvd.set", true);

    LOG_NRM("Set LID field reserved coded values");
    uint32_t cdw10 = getLogPgCmd->GetDword(10) & ~0xff;

    getLogPgCmd->SetDword(cdw10, 10);

    IO::SendAndReapCmdNot(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
            getLogPgCmd, "rsvd.set", true, CESTAT_SUCCESS);

    for (uint32_t lid = 0x4; lid <= 0x7f; ++lid) {
        work = cdw10 | lid;
        getLogPgCmd->SetDword(work, 10);

        IO::SendAndReapCmdNot(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
            getLogPgCmd, "rsvd.val.set", true, CESTAT_SUCCESS);
    }

    uint8_t css;
    gCtrlrConfig->GetCSS(css);
    if (css == 0) { /* NVM Command Set selected */
        for (uint32_t lid = 0x81; lid <= 0xbf; ++lid) {
            work = cdw10 | lid;
            getLogPgCmd->SetDword(work, 10);

            IO::SendAndReapCmdNot(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
                asq, acq, getLogPgCmd, "rsvd.val.set", true);
        }
    }

    else
    {
        throw new FrmwkEx(HERE, "Unsupported command set selected for test "
                "revision: %hhX", css);
    }
}

}   // namespace
