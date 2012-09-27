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
#include "mandatoryFW_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/io.h"
#include "../Cmds/getLogPage.h"

#define FW_SLOT_NUMD        (GetLogPage::FIRMSLOT_DATA_SIZE / 4)

namespace GrpAdminGetLogPgCmd {


MandatoryFW_r10b::MandatoryFW_r10b
    (string grpName, string testName) : Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Get mandatory FW Slot log page");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Create buffer, size=512, initVal=0x00; Issue GetLogPage cmd, LID=3. "
        "Issue the cmd multiple times such that NUMD loops X=[0...512], in "
        "steps of 4; reinit buffer = 0x00 each iteration, expect success and "
        "verify buffer bytes [(X+1) to 512] = 0x00.");
}


MandatoryFW_r10b::~MandatoryFW_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


MandatoryFW_r10b::
MandatoryFW_r10b(const MandatoryFW_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


MandatoryFW_r10b &
MandatoryFW_r10b::operator=(const MandatoryFW_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
MandatoryFW_r10b::RunnableCoreTest(bool preserve)
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
MandatoryFW_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     *  \endverbatim
     */
    string work;

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    LOG_NRM("Create get log page cmd and assoc some buffer memory");
    SharedGetLogPagePtr getLogPgCmd = SharedGetLogPagePtr(new GetLogPage());
    SharedMemBufferPtr getLogPageMem = SharedMemBufferPtr(new MemBuffer());
    send_64b_bitmask prpReq =
        (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);

    getLogPgCmd->SetLID(GetLogPage::LOGID_FW_SLOT);

    LOG_NRM("Issue cmd multiple times such that NUMD loops X=[0...512]");
    for (uint32_t numd = 1; numd <= FW_SLOT_NUMD; numd++) {
        LOG_NRM("Issue get log page for FW slot info with NUMD = %d", numd);
        getLogPageMem->Init(GetLogPage::FIRMSLOT_DATA_SIZE, true);
        getLogPgCmd->SetPrpBuffer(prpReq, getLogPageMem);
        getLogPgCmd->SetNUMD(numd - 1); // 0-based

        work = str(boost::format("FWSlot.NUMD.%d") % numd);
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
            getLogPgCmd, work, true);

        LOG_NRM("Verify the buffer's non-retrieved = 0x00");
        SharedMemBufferPtr cmdPayload = getLogPgCmd->GetRWPrpBuffer();
        uint32_t offset = (numd * 4);
        uint8_t *cmdPayloadBuff = (uint8_t *)cmdPayload->GetBuffer() + offset;
        for (; offset < GetLogPage::FIRMSLOT_DATA_SIZE; offset++) {
            LOG_NRM("Verify data at offset = 0x%X", offset);
            if (*cmdPayloadBuff != 0x0) {
                throw FrmwkEx(HERE, "Invalid data at buffer offset = 0x%08X, "
                    "value = 0x%08X", cmdPayloadBuff, *cmdPayloadBuff);
            }
            cmdPayloadBuff++;
        }
    }
}


}   // namespace
