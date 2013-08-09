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
#include "verifyMasking_r10b.h"
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


VerifyMasking_r10b::VerifyMasking_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section ?");
    mTestDesc.SetShort(     "Verify async events are masked.");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Reset ctrlr to cause a clearing of DUT state. Unable to perform this "
        "test unless Identify.AERL >= 2. Issue Identify.AERL async cmds. Ring "
        "doorbell #1 Identify.AERL times, then verify a single CE exists for "
        "1 async event,and, validate CE.DW0 for proper error.");
}


VerifyMasking_r10b::~VerifyMasking_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


VerifyMasking_r10b::
VerifyMasking_r10b(const VerifyMasking_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


VerifyMasking_r10b &
VerifyMasking_r10b::operator=(const VerifyMasking_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
VerifyMasking_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////
    if ((gInformative->GetIdentifyCmdCtrlr()->GetValue(IDCTRLRCAP_AERL) + 1)
        < 2) {
        LOG_NRM("DUT does not meet the runnable condition Identify.AERL >= 2");
        return RUN_FALSE;
    }

    // Choose to return one of these or create your own logic
    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive

}


void
VerifyMasking_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) none
     *  \endverbatim
     */
    uint32_t isrCount;
    uint32_t ceRemain;
    uint32_t numReaped;
    uint32_t numCE;

    LOG_NRM("Issue Identify.AERL to get Async Event Req Limit (AERL)");
    uint8_t nAerlimit = gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_AERL) + 1; // Convert to 1-based.

    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Create admin queues ACQ and ASQ for test lifetime");
    SharedACQPtr acq = SharedACQPtr(new ACQ(gDutFd));
    acq->Init(nAerlimit + 1); // one extra space than Q full condition

    SharedASQPtr asq = SharedASQPtr(new ASQ(gDutFd));
    asq->Init(nAerlimit + 1); // one extra space than Q full condition

    // All queues will use identical IRQ vector
    IRQ::SetAnySchemeSpecifyNum(1);

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Issue %d async event requests", nAerlimit);
    SendAsyncEventRequests(asq, nAerlimit);

    for (uint8_t nAer = 1; nAer <= nAerlimit; nAer++) {
        LOG_NRM("Ring doorbell for IOSQ #1");
        InvalidSQWriteDoorbell();
        sleep(5);
        if (nAer == 1) {
            LOG_NRM("Verify only 1 CE exists for invalid SQID doorbell write");
            if (acq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(1), 1, numCE,
                isrCount) == false) {
                acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName,
                    "acq.fail.reapinq1"), "Dump Entire ACQ");
                throw FrmwkEx(HERE, "1 CE expected in but found %d CEs", numCE);
            }
            SharedMemBufferPtr ceMem = SharedMemBufferPtr(new MemBuffer());
            if ((numReaped = acq->Reap(ceRemain, ceMem, isrCount, numCE, true))
                != 1) {
                acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName,
                    "acq.fail.reap1"), "Dump Entire ACQ");
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
        } else {
            LOG_NRM("Verify only 1 CE exists for %d cmds as "
                "async events are masked (GetLogPage not called)", nAer);
            if (acq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(1), 2, numCE,
                isrCount) == true) {
                acq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName,
                    "acq.fail.reapinq2"), "Dump Entire ACQ");
                throw FrmwkEx(HERE, "1 CE expected but found %d CEs", numCE);
            }
        }
    }
}


void
VerifyMasking_r10b::SendAsyncEventRequests(SharedASQPtr &asq, uint32_t nCmds)
{
    uint16_t uniqueId;
    string work;

    LOG_NRM("Create aync event request cmd");
    SharedAsyncEventReqPtr asyncEventReqCmd =
        SharedAsyncEventReqPtr(new AsyncEventReq());

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
VerifyMasking_r10b::InvalidSQWriteDoorbell()
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
VerifyMasking_r10b::ReadLogPage(SharedACQPtr &acq, SharedASQPtr &asq,
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
