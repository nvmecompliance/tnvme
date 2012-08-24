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

#include "manyCmdSubmit_r10b.h"
#include "globals.h"
#include "grpDefs.h"


namespace GrpQueues {


ManyCmdSubmit_r10b::ManyCmdSubmit_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4");
    mTestDesc.SetShort(     "Verify multiple cmds issued simultaneously.");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Search for 1 of the following namspcs to run test. Find 1st bare "
        "namspc, or find 1st meta namspc, or find 1st E2E namspc. Create IOQ "
        "pairs of size (CAP.MQES + 1), issue x simultaneous NVM write cmds, "
        "sending 1 block and approp supporting meta/E2E if necessary to the "
        "selected namspc at LBA 0, where x loops from 1 to Q full condition, "
        "then ring doorbell and verify all succeed.");
}


ManyCmdSubmit_r10b::~ManyCmdSubmit_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


ManyCmdSubmit_r10b::
ManyCmdSubmit_r10b(const ManyCmdSubmit_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


ManyCmdSubmit_r10b &
ManyCmdSubmit_r10b::operator=(const ManyCmdSubmit_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
ManyCmdSubmit_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
ManyCmdSubmit_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     *  \endverbatim
     */
    uint32_t nCmds;
    uint32_t isrCount;
    uint32_t ceRemain;
    uint32_t numReaped;
    uint32_t numCE;
    uint16_t uniqueId;

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    LOG_NRM("Determine the max IOQ entries supported");
    uint64_t ctrlCapReg;
    if (gRegisters->Read(CTLSPC_CAP, ctrlCapReg) == false)
        throw FrmwkEx(HERE, "Unable to determine MQES");
    uint32_t maxIOQEntries = (uint32_t)(ctrlCapReg & CAP_MQES);
    maxIOQEntries += 1;     // convert to 1-based.

    LOG_NRM("Create contig IOQ's");
    SharedIOCQPtr iocq = Queues::CreateIOCQContigToHdw(mGrpName,
        mTestName, CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, maxIOQEntries,
        false, IOCQ_CONTIG_GROUP_ID, true, 0);

    SharedIOSQPtr iosq = Queues::CreateIOSQContigToHdw(mGrpName,
        mTestName, CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, maxIOQEntries,
        false, IOSQ_CONTIG_GROUP_ID, IOQ_ID, 0);

    SharedWritePtr writeCmd = SetWriteCmd();

    uint32_t increment = 1;
    for (uint32_t x = 1; x < maxIOQEntries; x += increment) {
        LOG_NRM("Sending #%d simultaneous NVM write cmds to IOSQ", x);
        // Issue x simultaneous NVM write cmds.
        for (nCmds = 1; nCmds <= x; nCmds++)
            iosq->Send(writeCmd, uniqueId);
        iosq->Ring();

        // Variable wait time w.r.t "x" and expect all CE's to arrive in CQ.
        if (iocq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(x),
            x, numCE, isrCount) == false) {
            iocq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName,
                "iocq.reqpinq." + writeCmd->GetName()), "Dump Entire IOCQ");
            LogCEAndCQMetrics(iocq);
            throw FrmwkEx(HERE, "Unable to see CEs for issued cmds #%d", x);
        } else if (numCE != x) {
            iocq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName,
                "iocq.reqpinq." + writeCmd->GetName()), "Dump Entire IOCQ");
            LogCEAndCQMetrics(iocq);
            throw FrmwkEx(HERE, "The IOCQ should only have #%d CE's as a result "
                "of #%d simultaneous cmds but found #%d", x, x, numCE);
        }

        SharedMemBufferPtr ceMem = SharedMemBufferPtr(new MemBuffer());
        if ((numReaped = iocq->Reap(ceRemain, ceMem, isrCount, x, true)) != x) {
            iocq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName,
                "iocq.reap." + writeCmd->GetName()), "Dump Entire IOCQ");
            LogCEAndCQMetrics(iocq);
            throw FrmwkEx(HERE, "Unable to reap on IOCQ #%d. Reaped #%d of #%d",
                IOQ_ID, numReaped, x);
        }

        // Testing every cmd takes numerous hrs, so compromise.
        if ((maxIOQEntries - 1000) % x == 0)
            increment = 100;
        else if ((x % 1000) == 0)
            increment = 1000;
        else if ((maxIOQEntries - 100) % x == 0)
            increment = 10;
        else if ((x % 100) == 0)
            increment = 100;
        else if ((maxIOQEntries - 10) % x == 0)
            increment = 1;
        else if ((x % 10) == 0)
            increment = 10;

    }

    // Delete IOSQ before the IOCQ to comply with spec.
    Queues::DeleteIOSQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
        iosq, asq, acq);
    Queues::DeleteIOCQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
        iocq, asq, acq);
}


SharedWritePtr
ManyCmdSubmit_r10b::SetWriteCmd()
{
    Informative::Namspc namspcData = gInformative->Get1stBareMetaE2E();
    LOG_NRM("Processing write cmd using namspc id %d", namspcData.id);
    LBAFormat lbaFormat = namspcData.idCmdNamspc->GetLBAFormat();

    SharedMemBufferPtr dataPat = SharedMemBufferPtr(new MemBuffer());
    uint64_t lbaDataSize = namspcData.idCmdNamspc->GetLBADataSize();

    SharedWritePtr writeCmd = SharedWritePtr(new Write());
    send_64b_bitmask prpBitmask = (send_64b_bitmask)(MASK_PRP1_PAGE
        | MASK_PRP2_PAGE | MASK_PRP2_LIST);

    switch (namspcData.type) {
    case Informative::NS_BARE:
        dataPat->Init(lbaDataSize);
        break;
    case Informative::NS_METAS:
        dataPat->Init(lbaDataSize);
        if (gRsrcMngr->SetMetaAllocSize(lbaFormat.MS) == false)
            throw FrmwkEx(HERE);
        writeCmd->AllocMetaBuffer();
        break;
    case Informative::NS_METAI:
        dataPat->Init(lbaDataSize + lbaFormat.MS);
        break;
    case Informative::NS_E2ES:
    case Informative::NS_E2EI:
        throw FrmwkEx(HERE, "Deferring work to handle this case in future");
        break;
    }

    writeCmd->SetPrpBuffer(prpBitmask, dataPat);
    writeCmd->SetNSID(namspcData.id);
    writeCmd->SetNLB(0);

    return writeCmd;
}


void
ManyCmdSubmit_r10b::LogCEAndCQMetrics(SharedIOCQPtr iocq)
{
    struct nvme_gen_cq iocqMetrics = iocq->GetQMetrics();
    union CE ce = iocq->PeekCE(iocqMetrics.head_ptr);

    LOG_NRM("Logging Completion Element (CE)...");
    LOG_NRM("  CE DWORD0: 0x%08X", ce.t.dw0);
    LOG_NRM("  CE DWORD1: 0x%08X", ce.t.dw1);
    LOG_NRM("  CE DWORD2: 0x%08X", ce.t.dw2);
    LOG_NRM("  CE DWORD3: 0x%08X", ce.t.dw3);

    LOG_NRM("dnvme metrics pertaining to CQ ID: %d", iocqMetrics.q_id);
    LOG_NRM("  tail_ptr       = %d", iocqMetrics.tail_ptr);
    LOG_NRM("  head_ptr       = %d", iocqMetrics.head_ptr);
    LOG_NRM("  elements       = %d", iocqMetrics.elements);
    LOG_NRM("  irq_enabled    = %s", iocqMetrics.irq_enabled ? "T" : "F");
    LOG_NRM("  irq_no         = %d", iocqMetrics.irq_no);
    LOG_NRM("  pbit_new_entry = %d", iocqMetrics.pbit_new_entry);
}

}   // namespace
