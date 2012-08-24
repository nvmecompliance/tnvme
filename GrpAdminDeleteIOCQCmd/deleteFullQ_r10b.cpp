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

#include "deleteFullQ_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/irq.h"
#include "../Utils/queues.h"
#include "../Cmds/deleteIOSQ.h"
#include "../Cmds/deleteIOCQ.h"

#define MIN_REQ_IOQ_ENTRIES     4

namespace GrpAdminDeleteIOCQCmd {


DeleteFullQ_r10b::DeleteFullQ_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Delete an IOCQ that is full of cmds.");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Search for 1 of the following namspcs to run test. Find 1st bare "
        "namspc, or find 1st meta namspc, or find 1st E2E namspc. Create an "
        "IOCQ/IOSQ pair, both with QID = 1, with SQ num elements = 4 and "
        "CQ num elements = 2. Fill the IOSQ with write cmds sending 1 block "
        "and approp supporting meta/E2E if necessary to the selected namspc "
        "at LBA 0, ring the doorbell to cause the cmds to execute and fill "
        "up the IOCQ. Verify CE's arrive until CQ fills up. Issue a "
        "DeleteIOSQ cmd, then a DeleteIOCQ cmd and expect success for both. "
        "Run the entire test again but with IOQ's created with num SQ "
        "elements = (CAP.MQES + 1) and num CQ elements = (CAP.MQES + 1) / 2, "
        "expect identical outcome.");
}


DeleteFullQ_r10b::~DeleteFullQ_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


DeleteFullQ_r10b::
DeleteFullQ_r10b(const DeleteFullQ_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


DeleteFullQ_r10b &
DeleteFullQ_r10b::operator=(const DeleteFullQ_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
DeleteFullQ_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
DeleteFullQ_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * None.
     * \endverbatim
     */
    uint64_t ctrlCapReg;
    LOG_NRM("Determine the max IOQ entries supported");
    if (gRegisters->Read(CTLSPC_CAP, ctrlCapReg) == false)
        throw FrmwkEx(HERE, "Unable to determine MQES");
    uint32_t maxIOQEntries = (ctrlCapReg & CAP_MQES);
    maxIOQEntries += 1;      // convert to 1-based
    if (maxIOQEntries < MIN_REQ_IOQ_ENTRIES) {
        LOG_WARN("Desired to support >= %d elements in IOSQ for this test",
            MIN_REQ_IOQ_ENTRIES);
        return;
    }

    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Create admin queues ACQ and ASQ");
    SharedACQPtr acq = SharedACQPtr(new ACQ(gDutFd));
    acq->Init(5);

    SharedASQPtr asq = SharedASQPtr(new ASQ(gDutFd));
    asq->Init(5);

    // All queues will use identical IRQ vector
    IRQ::SetAnySchemeSpecifyNum(1);

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Setup element sizes for the IOQ's");
    gCtrlrConfig->SetIOCQES(gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_CQES) & 0xf);
    gCtrlrConfig->SetIOSQES(gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_SQES) & 0xf);

    LOG_NRM("Case 1: Delete full IOQ with IOSQ = %d and IOCQ = %d entries",
        MIN_REQ_IOQ_ENTRIES, (MIN_REQ_IOQ_ENTRIES / 2));
    DeleteFullIOQs(acq, asq, MIN_REQ_IOQ_ENTRIES);

    LOG_NRM("Case 2: Delete full IOQ with IOSQ = (CAP.MQES + 1) entries and"
        "IOCQ = ((CAP.MQES + 1) / 2) entries");
    DeleteFullIOQs(acq, asq, maxIOQEntries);
}


void
DeleteFullQ_r10b::DeleteFullIOQs(SharedACQPtr acq, SharedASQPtr asq,
    uint32_t numIOQEntries)
{
    LOG_NRM("Create IOCQ with entries #%d", (numIOQEntries / 2));
    SharedIOCQPtr iocq = Queues::CreateIOCQContigToHdw(mGrpName,
        mTestName, CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, (numIOQEntries / 2),
        false, IOCQ_GROUP_ID, true, 0);

    LOG_NRM("Create IOSQ with entries #%d", numIOQEntries);
    SharedIOSQPtr iosq = Queues::CreateIOSQContigToHdw(mGrpName,
        mTestName, CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, numIOQEntries,
        false, IOSQ_GROUP_ID, IOQ_ID, 0);

    LOG_NRM("Fill up IOQ's with entries #%d", (numIOQEntries - 1));
    SendCmdsToFillQsAndVerify(iosq, iocq, (numIOQEntries - 1));

    LOG_NRM("Delete IOSQ before the IOCQ to comply with spec.");
    Queues::DeleteIOSQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
        iosq, asq, acq);
    LOG_NRM("Delete IOCQ.");
    Queues::DeleteIOCQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
        iocq, asq, acq);
}


void
DeleteFullQ_r10b::SendCmdsToFillQsAndVerify(SharedIOSQPtr iosq,
    SharedIOCQPtr iocq, uint32_t nCmdsToSubmit)
{
    uint32_t numCE;
    uint32_t isrCount;
    uint16_t uniqueId;

    LOG_NRM("Prepare write cmd to be used for submitting to Q's.");
    SharedWritePtr writeCmd = CreateWriteCmd();

    LOG_NRM("Send #%d cmds to hdw via IOSQ 1", nCmdsToSubmit);
    for (uint32_t nCmds = 0; nCmds < nCmdsToSubmit; nCmds++)
        iosq->Send(writeCmd, uniqueId);
    iosq->Ring();

    uint32_t expCEs = (iocq->GetNumEntries() - 1);
    LOG_NRM("Verify that expected CE's #%d arrive in IOCQ 1", expCEs);
    if (iocq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(expCEs),
        expCEs, numCE, isrCount) == false) {
        iocq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName,
            "iocq.fail"), "Dump entire IOCQ");
        iosq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName,
            "iosq.fail"), "Dump entire IOSQ");
        throw FrmwkEx(HERE, "Unable to see expected CE's #%d", expCEs);
    }
}


SharedWritePtr
DeleteFullQ_r10b::CreateWriteCmd()
{
    LOG_NRM("Get fisrt Bare/Meta/E2E namespace");
    Informative::Namspc namspcData = gInformative->Get1stBareMetaE2E();
    LBAFormat lbaFormat = namspcData.idCmdNamspc->GetLBAFormat();
    uint64_t lbaDataSize = namspcData.idCmdNamspc->GetLBADataSize();

    LOG_NRM("Create write cmd using namspc id %d", namspcData.id);
    SharedWritePtr writeCmd = SharedWritePtr(new Write());

    LOG_NRM("Create write memory to use for writing data to media");
    SharedMemBufferPtr writeMem = SharedMemBufferPtr(new MemBuffer());
    send_64b_bitmask prpBitmask = (send_64b_bitmask)(MASK_PRP1_PAGE
        | MASK_PRP2_PAGE | MASK_PRP2_LIST);

    LOG_NRM("Allocate write memory based on namspc type #%d", namspcData.type);
    switch (namspcData.type) {
    case Informative::NS_BARE:
        writeMem->Init(lbaDataSize);
        break;
    case Informative::NS_METAS:
        writeMem->Init(lbaDataSize);
        if (gRsrcMngr->SetMetaAllocSize(lbaFormat.MS) == false)
            throw FrmwkEx(HERE);
        writeCmd->AllocMetaBuffer();
        break;
    case Informative::NS_METAI:
        writeMem->Init(lbaDataSize + lbaFormat.MS);
        break;
    case Informative::NS_E2ES:
    case Informative::NS_E2EI:
        throw FrmwkEx(HERE, "Deferring work to handle this case in future");
        break;
    }

    writeCmd->SetPrpBuffer(prpBitmask, writeMem);
    writeCmd->SetNSID(namspcData.id);
    writeCmd->SetNLB(0);

    return writeCmd;
}


}   // namespace

