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

#include "numDIsAdhered_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/io.h"
#include "../Cmds/getLogPage.h"

#define FIRM_SLOT_INFO_LID      0x03
#define NUMDW_ADHERED           ((514 / 4) / 2)
#define BUFFER_OFFSET           0x0
#define BUFFER_INIT_VAL         0xA5

namespace GrpAdminGetLogPgCmd {


NUMDIsAdhered_r10b::NUMDIsAdhered_r10b(string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Verify GetLogPage NUMD value is adhered");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Desiring to request half of a mandatory page and verify the NUMD "
        "was followed. Create buffer, size=512, initVal=0xA5; Issue "
        "GetLogPage cmd, LID=3, NUMD=((512/4)/2), i.e half this log page, "
        "expect success; verify the last half of buffer=0xA5.");
}


NUMDIsAdhered_r10b::~NUMDIsAdhered_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


NUMDIsAdhered_r10b::
NUMDIsAdhered_r10b(const NUMDIsAdhered_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


NUMDIsAdhered_r10b &
NUMDIsAdhered_r10b::operator=(const NUMDIsAdhered_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
NUMDIsAdhered_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    preserve = preserve;    // Suppress compiler error/warning
    return RUN_TRUE;        // This test is never destructive
}


void
NUMDIsAdhered_r10b::RunCoreTest()
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
    getLogPgCmd->SetNUMD(NUMDW_ADHERED - 1);  // 0-based
    getLogPgCmd->SetLID(FIRM_SLOT_INFO_LID);

    LOG_NRM("Set the offset into the buffer at 0x%04X", BUFFER_OFFSET);
    SharedMemBufferPtr getLogPageMem = SharedMemBufferPtr(new MemBuffer());
    getLogPageMem->InitOffset1stPage(GetLogPage::FIRMSLOT_DATA_SIZE,
        BUFFER_OFFSET, true, BUFFER_INIT_VAL);
    send_64b_bitmask prpReq =
        (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    getLogPgCmd->SetPrpBuffer(prpReq, getLogPageMem);

    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
        getLogPgCmd, "NUMD.adhered", true);

    LOG_NRM("Compare cmd buffer to verify the last half of buffer = 0xA5");
    SharedMemBufferPtr cmdPayload = getLogPgCmd->GetRWPrpBuffer();
    uint16_t offset = (NUMDW_ADHERED * 4);
    uint8_t *cmdPayloadBuff = (uint8_t *)cmdPayload->GetBuffer() + offset;
    for (; offset < (GetLogPage::FIRMSLOT_DATA_SIZE); offset++) {
        LOG_NRM("Verify data at offset = 0x%X", offset);
        if (*cmdPayloadBuff != BUFFER_INIT_VAL) {
            throw FrmwkEx(HERE, "NUMD not adhered at offset = 0x%08X, "
                "value = 0x%08X", cmdPayloadBuff, *cmdPayloadBuff);
        }
        cmdPayloadBuff++;
    }
}

}   // namespace
