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
#include "mandatoryErrInfo_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/io.h"
#include "../Cmds/getLogPage.h"

#define ERRINFO_LID         0x01
#define ERRINFO_NUMD        (GetLogPage::ERRINFO_DATA_SIZE / 4)

namespace GrpAdminGetLogPgCmd {


MandatoryErrInfo_r10b::MandatoryErrInfo_r10b
    (string grpName, string testName) : Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Get mandatory error info log page");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Determine X=(Identify.ELPE+1). Create a buffer, size=(X*64). Create "
		"GetLogPage cmd with LID=1. Issue the cmd multiple times while "
		"looping NUMD such that the number of log entries ranges from [0..X], "
		"expect success. For each loop init buffer=0, and verify the buffer's "
		"non-retrieved log entries = 0x00.");
}


MandatoryErrInfo_r10b::~MandatoryErrInfo_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


MandatoryErrInfo_r10b::
MandatoryErrInfo_r10b(const MandatoryErrInfo_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


MandatoryErrInfo_r10b &
MandatoryErrInfo_r10b::operator=(const MandatoryErrInfo_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
MandatoryErrInfo_r10b::RunnableCoreTest(bool preserve)
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
MandatoryErrInfo_r10b::RunCoreTest()
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

    ConstSharedIdentifyPtr idCtrlrStruct = gInformative->GetIdentifyCmdCtrlr();
    uint8_t X = idCtrlrStruct->GetValue(IDCTRLRCAP_ELPE) + 1;
    LOG_NRM("Identify controller ELPE = %d (1-based)", X);

    LOG_NRM("Create get log page cmd and assoc some buffer memory");
    SharedGetLogPagePtr getLogPgCmd = SharedGetLogPagePtr(new GetLogPage());

    LOG_NRM("Create memory buffer for log page to request error information");
    SharedMemBufferPtr getLogPageMem = SharedMemBufferPtr(new MemBuffer());
    send_64b_bitmask prpReq =
        (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);

    LOG_NRM("Get log page to request error information");
    getLogPgCmd->SetLID(ERRINFO_LID);

    // loop for all log entries supported by controller
    for (uint32_t numd = ERRINFO_NUMD; numd <= (X * ERRINFO_NUMD);
        numd += ERRINFO_NUMD) {
        LOG_NRM("Issue Get log page cmd with NUMD = %d and log entries = %d",
            numd, (numd/ERRINFO_NUMD));

        getLogPageMem->Init(GetLogPage::ERRINFO_DATA_SIZE * X, true);
        getLogPgCmd->SetPrpBuffer(prpReq, getLogPageMem);
        getLogPgCmd->SetNUMD(numd - 1); // 0-based

        work = str(boost::format("logEnties%d") % (numd / ERRINFO_NUMD));
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
            getLogPgCmd, work, true);

        // Verify the buffer's non-retrieved log entries = 0x00.
        SharedMemBufferPtr cmdPayload = getLogPgCmd->GetRWPrpBuffer();
        uint32_t offset = (numd * 4);
        uint8_t *cmdPayloadBuff = (uint8_t *)cmdPayload->GetBuffer() + offset;
        for (; offset < (X * GetLogPage::ERRINFO_DATA_SIZE); offset++) {
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
