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

#include "sqcqSizeMismatch_r10b.h"
#include "grpDefs.h"
#include "../Utils/queues.h"
#include "../Utils/io.h"
#include "../Utils/kernelAPI.h"


namespace GrpQueues {


SQCQSizeMismatch_r10b::SQCQSizeMismatch_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4");
    mTestDesc.SetShort(     "Create IOCQ/IOSQ pairs while mismatch Q sizes.");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Search for 1 of the following namspcs to run test. Find 1st bare "
        "namspc, or find 1st meta namspc, or find 1st E2E namspc. Create X "
        "IOSQ/IOCQ pairs, where X = max num IOQ's the DUT supports. Range "
        "the size of each IOSQ from 2 to (X+2), and range the sizeof the "
        "correlating IOCQ's from (X+2) to 2. If the DUT supports 65536 "
        "IOQ’s, then 2 IOQ’s of each type must be max’d at (CAP.MQES+1). "
        "After all IOQ’s have been created, for each IOQ pair, fill up "
        "completely the IOSQ's with write cmds sending 1 block and approp "
        "supporting meta/E2E if necessary to the selected namspc at LBA 0, "
        "data pattern is a don't care. Reap all cmds successfully from the "
        "associated IOCQ. Verify the associated IOSQ for each CE reaped "
        "from all IOCQ's.");
}


SQCQSizeMismatch_r10b::~SQCQSizeMismatch_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


SQCQSizeMismatch_r10b::
SQCQSizeMismatch_r10b(const SQCQSizeMismatch_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


SQCQSizeMismatch_r10b &
SQCQSizeMismatch_r10b::operator=(const SQCQSizeMismatch_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
SQCQSizeMismatch_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
SQCQSizeMismatch_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     *  \endverbatim
     */
    uint16_t uniqueId;
    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    uint64_t maxIOQEntries;
    LOG_NRM("Determine the max IOQ entries supported");
    if (gRegisters->Read(CTLSPC_CAP, maxIOQEntries) == false)
        throw FrmwkEx(HERE, "Unable to determine MQES");
    maxIOQEntries &= CAP_MQES;
    maxIOQEntries += 1;      // convert to 1-based

    SharedWritePtr writeCmd = SetWriteCmd();

    vector<SharedIOSQPtr> IOSQVec;
    vector<SharedIOCQPtr> IOCQVec;

    uint32_t maxIOQSupport = MIN(gInformative->GetFeaturesNumOfIOSQs(),
        gInformative->GetFeaturesNumOfIOCQs());
    uint32_t NumEntriesIOSQ = 2; // IOSQ range: 2 to X + 2
    uint32_t NumEntriesIOCQ = ((maxIOQSupport + 2) > maxIOQEntries) ?
        maxIOQEntries : (maxIOQSupport + 2); // IOCQ range: X + 2 to 2

    // Create all supported  queues.
    for (uint32_t ioqId = 1; ioqId <= maxIOQSupport; ioqId++) {
        LOG_NRM("Creating IOQs with IDs #%d of maximum IDs %d",
            ioqId, maxIOQSupport);
        SharedIOCQPtr iocq = Queues::CreateIOCQContigToHdw(mGrpName,
            mTestName, CALC_TIMEOUT_ms(1), asq, acq, ioqId, NumEntriesIOCQ,
            false, IOCQ_CONTIG_GROUP_ID, false, 0);
        SharedIOSQPtr iosq = Queues::CreateIOSQContigToHdw(mGrpName,
            mTestName, CALC_TIMEOUT_ms(1), asq, acq, ioqId, NumEntriesIOSQ,
            false, IOSQ_CONTIG_GROUP_ID, ioqId, 0);
        IOSQVec.push_back(iosq);
        IOCQVec.push_back(iocq);

        if (NumEntriesIOSQ < maxIOQEntries) {
            NumEntriesIOSQ++;
            NumEntriesIOCQ--;
        }
    }

    vector <SharedIOSQPtr>::iterator iosq;
    vector <SharedIOCQPtr>::iterator iocq;
    LOG_NRM("Send cmds until all SQs fill up.");
    for (iosq = IOSQVec.begin(); iosq != IOSQVec.end(); iosq++) {
        for (uint32_t numCmds = 1; numCmds < ((*iosq)->GetNumEntries());
            numCmds++) {
            (*iosq)->Send(writeCmd, uniqueId);
        }
        (*iosq)->Ring();
    }

    LOG_NRM("Reap and verify all cmds submitted.");
    iosq = IOSQVec.begin();
    for (iocq = IOCQVec.begin(); iocq != IOCQVec.end(); iocq++, iosq++)
        ReapVerifyOnCQ(*iocq, *iosq);

    // Clean up after the test by deleting all the Q's created.
    iocq = IOCQVec.begin();
    for (iosq = IOSQVec.begin(); iosq != IOSQVec.end(); iosq++, iocq++) {
        // Delete IOSQ before the IOCQ to comply with spec.
        Queues::DeleteIOSQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
            *iosq, asq, acq);
        Queues::DeleteIOCQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
            *iocq, asq, acq);
    }

}


SharedWritePtr
SQCQSizeMismatch_r10b::SetWriteCmd()
{
    Informative::Namspc namspcData = gInformative->Get1stBareMetaE2E();
    LOG_NRM("Processing write cmd using namspc id %d", namspcData.id);
    LBAFormat lbaFormat = namspcData.idCmdNamspc->GetLBAFormat();

    LOG_NRM("Create data pattern to write to media");
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
SQCQSizeMismatch_r10b::ReapVerifyOnCQ(SharedIOCQPtr iocq, SharedIOSQPtr iosq)
{
    uint32_t numCE;
    uint32_t ceRemain;
    uint32_t numReaped;
    uint32_t isrCount;

    SharedMemBufferPtr ceMemIOCQ = SharedMemBufferPtr(new MemBuffer());
    for (uint32_t nCmds = 1; nCmds < iosq->GetNumEntries(); nCmds++) {
        LOG_NRM("Wait for the CE to arrive in IOCQ");
        if (iocq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(1), 1, numCE,
            isrCount) == false) {
            iocq->Dump(
                FileSystem::PrepDumpFile(mGrpName, mTestName, "iocq", "reapInq"),
                "Unable to see any CE's in IOCQ, dump entire CQ contents");
            throw FrmwkEx(HERE, "Unable to see completion of cmd");
        }

        LOG_NRM("The CQ's metrics B4 reaping holds head_ptr needed");
        struct nvme_gen_cq iocqMetrics = iocq->GetQMetrics();
        KernelAPI::LogCQMetrics(iocqMetrics);

        if ((numReaped = iocq->Reap(ceRemain, ceMemIOCQ, isrCount, 1, true))
            != 1) {
            iocq->Dump(
                FileSystem::PrepDumpFile(mGrpName, mTestName, "iocq", "reap"),
                "Unable to see any CE's in IOCQ, dump entire CQ contents");
            throw FrmwkEx(HERE, "Verified there was 1 CE, but reaping failed");
        }

        LOG_NRM("The reaped CE is...");
        iocq->LogCE(iocqMetrics.head_ptr);

        union CE ce = iocq->PeekCE(iocqMetrics.head_ptr);
        ProcessCE::Validate(ce);  // throws upon error

        if (ce.n.SQID != iosq->GetQId()) {
            iocq->Dump(
                FileSystem::PrepDumpFile(mGrpName, mTestName, "iocq", "sqId"),
                "Wrong SQID in the CE of IOCQ, dump entire CQ contents");
            throw FrmwkEx(HERE, "Invalid SQID %d in CE, expected SQID", ce.n.SQID,
                iosq->GetQId());
        }
    }
}

}   // namespace
