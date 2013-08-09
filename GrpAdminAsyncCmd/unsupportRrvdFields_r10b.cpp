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

#include <string.h>
#include <boost/format.hpp>
#include "unsupportRrvdFields_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/irq.h"
#include "../Utils/io.h"
#include "../Cmds/asyncEventReq.h"
#include "../Cmds/asyncEventReqDefs.h"
#include "../Cmds/getLogPage.h"


#define SQ0TBDL     0x1000


namespace GrpAdminAsyncCmd {


UnsupportRrvdFields_r10b::UnsupportRrvdFields_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section ?");
    mTestDesc.SetShort(     "Set unsupported/rsvd fields in cmd");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Reset ctrlr to cause a clearing of DUT state. Issue 1 async cmd. "
        "Delay 5 sec, verify no CE exists in ACQ. Ring doorbell for IOSQ # 1, "
        "reap a successful CE from the ASQ, validate CE.DW0 for proper error. "
        "Read error event log page to clear async event. Unsupported DW's and "
        "rsvd fields are treated identical, the recipient shall not check "
        "their value. Issue same cmd setting all unsupported/rsvd fields, "
        "expect success. Set: DW0_b15:10, DW2, DW3, DW4, DW5, DW6, DW7, DW8, "
        "DW9, DW10, DW11, DW12, DW13, DW14, DW15. Validate CE.DW0 for ringing "
        "non-existent doorbell. Read error event log page to clear "
        "async event.");
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

    // Choose to return one of these or create your own logic
    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive

}


void
UnsupportRrvdFields_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) none
     *  \endverbatim
     */
    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Create admin queues ACQ and ASQ for test lifetime");
    SharedACQPtr acq = SharedACQPtr(new ACQ(gDutFd));
    acq->Init(5);

    SharedASQPtr asq = SharedASQPtr(new ASQ(gDutFd));
    asq->Init(5);

    // All queues will use identical IRQ vector
    IRQ::SetAnySchemeSpecifyNum(1);

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);

    // Test async events without setting the rsvd bits in the cmd.
    TestAsyncEvents(acq, asq, false);

    // Test async events by setting the rsvd bits in the cmd.
    TestAsyncEvents(acq, asq, true);

}


void
UnsupportRrvdFields_r10b::TestAsyncEvents(SharedACQPtr &acq, SharedASQPtr &asq,
    bool rsvd)
{
    uint32_t isrCount;
    uint32_t ceRemain;
    uint32_t numReaped;
    uint32_t numCE;

    SendAsyncEventRequests(asq, 1, rsvd);

    sleep(5);
    if (acq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(1), 1, numCE, isrCount)
        == true) {
        acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "acq.fail1"),
            "Dump Entire ACQ");
        throw FrmwkEx(HERE, "0 CE's expected in ACQ but found %d CE's", numCE);
    }

    InvalidSQWriteDoorbell();
    sleep(1);
    if (acq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(1), 1, numCE, isrCount)
        == false) {
        acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "acq.fail2"),
            "Dump Entire ACQ");
        throw FrmwkEx(HERE, "1 CE expected in ACQ but found %d CE's", numCE);
    }

    SharedMemBufferPtr ceMem = SharedMemBufferPtr(new MemBuffer());
    if ((numReaped = acq->Reap(ceRemain, ceMem, isrCount, numCE, true)) != 1) {
        acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "acq.fail3"),
            "Dump Entire ACQ");
        throw FrmwkEx(HERE, "Unable to reap on ACQ");
    }

    union CE *ce = (union CE *)ceMem->GetBuffer();

    if (ce->n.async.asyncEventType != EVENT_TYPE_ERROR_STS) {
        throw FrmwkEx(HERE, "Invalid async event error status, "
            "(Expected : Received) :: (%d : %d)", EVENT_TYPE_ERROR_STS,
            ce->n.async.asyncEventType);
    } else if (ce->n.async.asyncEventInfo != ERR_STS_INVALID_SQ) {
        throw FrmwkEx(HERE, "Invalid async event info, "
            "(Expected : Received) :: (%d : %d)", ERR_STS_INVALID_SQ,
            ce->n.async.asyncEventInfo);
    }
    LOG_NRM("Associated Log page = %d", ce->n.async.assocLogPage);

    ReadLogPage(acq, asq, ce->n.async.assocLogPage);
}


void
UnsupportRrvdFields_r10b::SendAsyncEventRequests(SharedASQPtr &asq,
    uint32_t nCmds, bool rsvd)
{
    uint16_t uniqueId;
    string work;

    LOG_NRM("Create aync event request cmd");
    SharedAsyncEventReqPtr asyncEventReqCmd =
        SharedAsyncEventReqPtr(new AsyncEventReq());

    if (rsvd == true) {
        LOG_NRM("Set all cmd's rsvd bits");
        uint32_t work = asyncEventReqCmd->GetDword(0);
        work |= 0x0000fc00;      // Set DW0_b15:10 bits
        asyncEventReqCmd->SetDword(work, 0);

        for (uint32_t dw = 2; dw <= 15; dw++)
            asyncEventReqCmd->SetDword(0xffffffff, dw);
    } else {
        LOG_NRM("Reserved bits in the cmd are not set");
    }

    for (uint32_t i = 0; i < nCmds; i++) {
        LOG_NRM("Send the async event request cmd to hdw via ASQ");
        asq->Send(asyncEventReqCmd, uniqueId);
        work = str(boost::format("asyncEventReq.%d") % i);
        asq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName,
            "asq", work), "Before doorbell ring");
        asq->Ring();
    }
}


void
UnsupportRrvdFields_r10b::InvalidSQWriteDoorbell()
{
    uint64_t value;
    if (gRegisters->Read((CtlSpc)CTLSPC_CAP, value) == false)
        throw FrmwkEx(HERE);

    value &= CAP_DSTRD;
    uint32_t capDstrd = value >> 32;
    uint32_t sq1Doorbell = SQ0TBDL + (2 * 4 << capDstrd);
    LOG_NRM("SQ 1 doorbell in controller space = 0x%X", sq1Doorbell);

    value = 0x1;
    if (gRegisters->Write(NVMEIO_BAR01, sizeof(uint32_t), sq1Doorbell,
        (uint8_t *)&value) == false)
          throw FrmwkEx(HERE);
}


void
UnsupportRrvdFields_r10b::ReadLogPage(SharedACQPtr &acq, SharedASQPtr &asq,
    uint8_t logId)
{
    string work;

    LOG_NRM("Reading log page with LID = %d to clear the event mask", logId);
    ConstSharedIdentifyPtr idCtrlrStruct = gInformative->GetIdentifyCmdCtrlr();
    uint8_t X = idCtrlrStruct->GetValue(IDCTRLRCAP_ELPE) + 1;
    LOG_NRM("Identify controller ELPE = %d (1-based)", X);

    LOG_NRM("Create get log page cmd and assoc some buffer memory");
    SharedGetLogPagePtr getLogPgCmd = SharedGetLogPagePtr(new GetLogPage());

    LOG_NRM("Create memory buffer for log page to request error information");
    SharedMemBufferPtr getLogPageMem = SharedMemBufferPtr(new MemBuffer());
    send_64b_bitmask prpReq =
        (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);

    LOG_NRM("Get log page to request error information logId = %d", logId);
    getLogPgCmd->SetLID(logId);
    getLogPageMem->Init(GetLogPage::ERRINFO_DATA_SIZE * X, true);
    getLogPgCmd->SetPrpBuffer(prpReq, getLogPageMem);
    getLogPgCmd->SetNUMD((GetLogPage::ERRINFO_DATA_SIZE * X / 4) - 1); //0-based

    work = str(boost::format("ErrorLog%d") % (uint32_t)logId);
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
        getLogPgCmd, work, true);
}


}   // namespace
