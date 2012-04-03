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

#include "qIdVariations_r10b.h"
#include "grpDefs.h"
#include "../Utils/queues.h"
#include "../Utils/io.h"
#include "../Utils/kernelAPI.h"


namespace GrpQueues {


QIDVariations_r10b::QIDVariations_r10b(int fd, string grpName,
    string testName, ErrorRegs errRegs) :
    Test(fd, grpName, testName, SPECREV_10b, errRegs)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Vary the QID pair's.");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Search for 1 of the following namspcs to run test. Find 1st bare "
        "namspc, or find 1st meta namspc, or find 1st E2E namspc. Create X "
        "IOSQ/IOCQ pairs, where X = max num IOQ's the DUT supports. Range the "
        "size of each IOSQ from 2 to (X+2), and range the sizeof the "
        "correlating IOCQ's from (X+2) to 2. If the DUT supports 65536 IOQ’s, "
        "then 2 IOQ’s of each type must be max’d at (CAP.MQES + 1). Create all "
        "IOCQ's with QID's ranging from 1 to X, then Create the correlating "
        "IOSQ's with QID's ranging from X to 1. After all IOQ’s have been "
        "created, for each IOQ pair, fill up completely the IOSQ's with write "
        "cmds sending 1 block and approp supporting meta/E2E if necessary to "
        "the selected namspc at LBA 0, data pattern is a don't care. Reap all "
        "cmds successfully from the associated IOCQ. Verify the associated "
        "IOSQ for each CE reaped from all IOCQ's.");
}


QIDVariations_r10b::~QIDVariations_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


QIDVariations_r10b::
QIDVariations_r10b(const QIDVariations_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


QIDVariations_r10b &
QIDVariations_r10b::operator=(const QIDVariations_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


void
QIDVariations_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     *  \endverbatim
     */

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    uint64_t maxIOQEntries;
    // Determine the max IOQ entries supported
    if (gRegisters->Read(CTLSPC_CAP, maxIOQEntries) == false)
        throw FrmwkEx("Unable to determine MQES");
    maxIOQEntries &= CAP_MQES;
    maxIOQEntries += 1;      // convert to 1-based

    SharedWritePtr writeCmd = SetWriteCmd();

    uint32_t maxIOQSupport = MIN(gInformative->GetFeaturesNumOfIOSQs(),
        gInformative->GetFeaturesNumOfIOCQs());
    uint32_t NumEntriesIOSQ = 2; // IOSQ range: 2 to X + 2
    uint32_t NumEntriesIOCQ = ((maxIOQSupport + 2) > maxIOQEntries) ?
        maxIOQEntries : (maxIOQSupport + 2); // IOCQ range: X + 2 to 2

    uint8_t iocqes = (gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_CQES) & 0xf);
    uint8_t iosqes = (gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_SQES) & 0xf);

    vector<SharedIOSQPtr> IOSQVec;
    vector<SharedIOCQPtr> IOCQVec;
    // Create all supported  queues.
    for (uint32_t ioqId = 1; ioqId <= maxIOQSupport; ioqId++) {
        SharedIOCQPtr iocq;
        SharedIOSQPtr iosq;
        if (Queues::SupportDiscontigIOQ() == true) {
            SharedMemBufferPtr iocqBackedMem =
                SharedMemBufferPtr(new MemBuffer());
            iocqBackedMem->InitOffset1stPage
                ((NumEntriesIOCQ * (1 << iocqes)), 0, true);
            iocq = Queues::CreateIOCQDiscontigToHdw(mGrpName, mTestName,
                DEFAULT_CMD_WAIT_ms, asq, acq, ioqId, NumEntriesIOCQ,
                false, IOCQ_CONTIG_GROUP_ID, false, 0, iocqBackedMem);

            SharedMemBufferPtr iosqBackedMem =
                SharedMemBufferPtr(new MemBuffer());
            iosqBackedMem->InitOffset1stPage
                ((NumEntriesIOSQ * (1 << iosqes)), 0,true);
            iosq = Queues::CreateIOSQDiscontigToHdw(mGrpName, mTestName,
                DEFAULT_CMD_WAIT_ms, asq, acq, ((maxIOQSupport - ioqId) + 1),
                NumEntriesIOSQ, false, IOSQ_CONTIG_GROUP_ID, ioqId, 0,
                iosqBackedMem);
        } else {
            iocq = Queues::CreateIOCQContigToHdw(mGrpName,
                mTestName, DEFAULT_CMD_WAIT_ms, asq, acq, ioqId, NumEntriesIOCQ,
                false, IOCQ_CONTIG_GROUP_ID, false, 0);
            iosq = Queues::CreateIOSQContigToHdw(mGrpName,
                mTestName, DEFAULT_CMD_WAIT_ms, asq, acq,
                ((maxIOQSupport - ioqId) + 1), NumEntriesIOSQ, false,
                IOSQ_CONTIG_GROUP_ID, ioqId, 0);
        }

        IOSQVec.push_back(iosq);
        IOCQVec.push_back(iocq);

        if (NumEntriesIOSQ < maxIOQEntries) {
            NumEntriesIOSQ++;
            NumEntriesIOCQ--;
        }
    }

    vector <SharedIOSQPtr>::iterator iosq;
    vector <SharedIOCQPtr>::iterator iocq;
    // Send cmds until all SQs fill up.
    for (iosq = IOSQVec.begin(); iosq != IOSQVec.end(); iosq++) {
        for (uint32_t numCmds = 1; numCmds < ((*iosq)->GetNumEntries());
            numCmds++) {
            (*iosq)->Send(writeCmd);
        }
        (*iosq)->Ring();
    }

    // Reap and verify all cmds submitted.
    iosq = IOSQVec.begin();
    for (iocq = IOCQVec.begin(); iocq != IOCQVec.end(); iocq++, iosq++)
        ReapVerifyOnCQ(*iocq, *iosq);

    // Clean up after the test by deleting all the Q's created.
    iocq = IOCQVec.begin();
    for (iosq = IOSQVec.begin(); iosq != IOSQVec.end(); iosq++, iocq++) {
        // Delete IOSQ before the IOCQ to comply with spec.
        Queues::DeleteIOSQToHdw(mGrpName, mTestName, DEFAULT_CMD_WAIT_ms,
            *iosq, asq, acq);
        Queues::DeleteIOCQToHdw(mGrpName, mTestName, DEFAULT_CMD_WAIT_ms,
            *iocq, asq, acq);
    }

}


SharedWritePtr
QIDVariations_r10b::SetWriteCmd()
{
    Informative::Namspc namspcData = gInformative->Get1stBareMetaE2E();
    LOG_NRM("Processing write cmd using namspc id %d", namspcData.id);
    if (namspcData.type != Informative::NS_BARE) {
        LBAFormat lbaFormat = namspcData.idCmdNamspc->GetLBAFormat();
        if (gRsrcMngr->SetMetaAllocSize(lbaFormat.MS) == false)
            throw FrmwkEx();
    }

    LOG_NRM("Create data pattern to write to media");
    SharedMemBufferPtr dataPat = SharedMemBufferPtr(new MemBuffer());
    uint64_t lbaDataSize = namspcData.idCmdNamspc->GetLBADataSize();
    dataPat->Init(lbaDataSize);

    SharedWritePtr writeCmd = SharedWritePtr(new Write());
    send_64b_bitmask prpBitmask = (send_64b_bitmask)(MASK_PRP1_PAGE
        | MASK_PRP2_PAGE | MASK_PRP2_LIST);

    if (namspcData.type == Informative::NS_META) {
        writeCmd->AllocMetaBuffer();
    } else if (namspcData.type == Informative::NS_E2E) {
        writeCmd->AllocMetaBuffer();
        LOG_ERR("Deferring E2E namspc work to the future");
        throw FrmwkEx("Need to add CRC's to correlate to buf pattern");
    }

    writeCmd->SetPrpBuffer(prpBitmask, dataPat);
    writeCmd->SetNSID(namspcData.id);
    writeCmd->SetNLB(0);

    return writeCmd;
}


void
QIDVariations_r10b::ReapVerifyOnCQ(SharedIOCQPtr iocq, SharedIOSQPtr iosq)
{
    uint32_t numCE;
    uint32_t ceRemain;
    uint32_t numReaped;
    uint32_t isrCount;

    SharedMemBufferPtr ceMemIOCQ = SharedMemBufferPtr(new MemBuffer());
    for (uint32_t nCmds = 1; nCmds < iosq->GetNumEntries(); nCmds++) {
        LOG_NRM("Wait for the CE to arrive in IOCQ");
        if (iocq->ReapInquiryWaitSpecify(DEFAULT_CMD_WAIT_ms, 1, numCE,
            isrCount) == false) {
            iocq->Dump(
                FileSystem::PrepLogFile(mGrpName, mTestName, "iocq", "reapInq"),
                "Unable to see any CE's in IOCQ, dump entire CQ contents");
            throw FrmwkEx("Unable to see completion of cmd");
        }

        LOG_NRM("The CQ's metrics B4 reaping holds head_ptr needed");
        struct nvme_gen_cq iocqMetrics = iocq->GetQMetrics();
        KernelAPI::LogCQMetrics(iocqMetrics);

        if ((numReaped = iocq->Reap(ceRemain, ceMemIOCQ, isrCount, 1, true))
            != 1) {
            iocq->Dump(
                FileSystem::PrepLogFile(mGrpName, mTestName, "iocq", "reap"),
                "Unable to see any CE's in IOCQ, dump entire CQ contents");
            throw FrmwkEx("Verified there was 1 CE, but reaping failed");
        }

        LOG_NRM("The reaped CE is...");
        iocq->LogCE(iocqMetrics.head_ptr);

        union CE ce = iocq->PeekCE(iocqMetrics.head_ptr);
        ProcessCE::Validate(ce);  // throws upon error

        if (ce.n.SQID != iosq->GetQId()) {
            iocq->Dump(
                FileSystem::PrepLogFile(mGrpName, mTestName, "iocq", "sqId"),
                "Wrong SQID in the CE of IOCQ, dump entire CQ contents");
            throw FrmwkEx("Invalid SQID %d in CE, expected SQID", ce.n.SQID,
                iosq->GetQId());
        }
    }
}

}   // namespace
