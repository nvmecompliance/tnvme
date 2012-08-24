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

#include "ioqRollChkDiff_r10b.h"
#include "grpDefs.h"
#include "../Utils/io.h"
#include "../Utils/kernelAPI.h"

namespace GrpQueues {


IOQRollChkDiff_r10b::IOQRollChkDiff_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4");
    mTestDesc.SetShort(     "Validate IOQ doorbell rollover when IOQ's different size");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Search for 1 of the following namspcs to run test. Find 1st bare "
        "namspc, or find 1st meta namspc, or find 1st E2E namspc. Create an "
        "IOSQ/IOCQ pair of size 2 and (CAP.MQES + 1); however the IOSQ starts "
        "with max size while the IOCQ starts with min size. Issue "
        "(max Q size plus 2) generic NVM write cmds, sending 1 block and "
        "approp supporting meta/E2E if necessary to the selected namspc at "
        "LBA 0, to fill and rollover the Q's, reaping each cmd as one is "
        "submitted, verify each CE.SQID and  CE.SQHD is correct while "
        "filling. Verify IOSQ tail_ptr = <calc_based_on_IOSQ_size>, IOCQ "
        "head_ptr = <calc_based_on_IOCQ_size>");
}


IOQRollChkDiff_r10b::~IOQRollChkDiff_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


IOQRollChkDiff_r10b::
IOQRollChkDiff_r10b(const IOQRollChkDiff_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


IOQRollChkDiff_r10b &
IOQRollChkDiff_r10b::operator=(const IOQRollChkDiff_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
IOQRollChkDiff_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
IOQRollChkDiff_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     *  \endverbatim
     */
    uint64_t maxIOQEntries;
    // Determine the max IOQ entries supported
    if (gRegisters->Read(CTLSPC_CAP, maxIOQEntries) == false)
        throw FrmwkEx(HERE, "Unable to determine MQES");
    maxIOQEntries &= CAP_MQES;
    maxIOQEntries += 1;      // convert to 1-based

    LOG_NRM("IOSQ Max entries, IOCQ Min entries");
    IOQRollChkDiff((uint32_t)maxIOQEntries, 2);

    LOG_NRM("IOSQ Min entries, IOCQ Max entries");
    IOQRollChkDiff(2, (uint32_t)maxIOQEntries);
}


void
IOQRollChkDiff_r10b::IOQRollChkDiff(uint32_t numEntriesIOSQ,
    uint32_t numEntriesIOCQ)
{
    uint16_t uniqueId;

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    SharedIOCQPtr iocq = Queues::CreateIOCQContigToHdw(mGrpName,
        mTestName, CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, numEntriesIOCQ,
        false, IOCQ_CONTIG_GROUP_ID, false, 1);

    SharedIOSQPtr iosq = Queues::CreateIOSQContigToHdw(mGrpName,
        mTestName, CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, numEntriesIOSQ,
        false, IOSQ_CONTIG_GROUP_ID, IOQ_ID, 0);

    LOG_NRM("(IOCQ Size, IOSQ Size)=(%d,%d)", iocq->GetNumEntries(),
        iosq->GetNumEntries());

    SharedWritePtr writeCmd = SetWriteCmd();

    LOG_NRM("Send #%d cmds to hdw via the contiguous IOSQ #%d",
        MAX(iosq->GetNumEntries(), iocq->GetNumEntries()) + 2,
        iosq->GetQId());
    for (uint32_t numEntries = 0; numEntries < (uint32_t)(MAX
        (iosq->GetNumEntries(), iocq->GetNumEntries()) + 2);
        numEntries++) {
        LOG_NRM("Processing #%d entries", numEntries);
        iosq->Send(writeCmd, uniqueId);
        iosq->Ring();
        ReapAndVerifyCE(iocq, (numEntries + 1) % iosq->GetNumEntries());
    }
    VerifyQPointers(iosq, iocq);

    LOG_NRM("Delete IOSQ before the IOCQ to comply with spec.");
    Queues::DeleteIOSQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
        iosq, asq, acq);
    Queues::DeleteIOCQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
        iocq, asq, acq);
}


SharedWritePtr
IOQRollChkDiff_r10b::SetWriteCmd()
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
IOQRollChkDiff_r10b::ReapAndVerifyCE(SharedIOCQPtr iocq, uint32_t expectedVal)
{
    uint32_t numCE;
    uint32_t ceRemain;
    uint32_t numReaped;
    uint32_t isrCount;

    LOG_NRM("Wait for the CE to arrive in IOCQ %d", iocq->GetQId());
    if (iocq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(1), 1, numCE, isrCount)
        == false) {

        iocq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "iocq",
            "reapInq"),
            "Unable to see any CE's in IOCQ, dump entire CQ contents");
        throw FrmwkEx(HERE, "Unable to see completion of cmd");
    } else if (numCE != 1) {
        throw FrmwkEx(HERE, "The IOCQ should only have 1 CE as a result of a cmd");
    }

    LOG_NRM("The CQ's metrics before reaping holds head_ptr");
    struct nvme_gen_cq iocqMetrics = iocq->GetQMetrics();

    LOG_NRM("Reaping CE from IOCQ, requires memory to hold reaped CE");
    SharedMemBufferPtr ceMemIOCQ = SharedMemBufferPtr(new MemBuffer());
    if ((numReaped = iocq->Reap(ceRemain, ceMemIOCQ, isrCount, numCE, true))
        != 1) {

        throw FrmwkEx(HERE, 
            "Verified there was 1 CE, but reaping produced %d", numReaped);
    }

    union CE ce = iocq->PeekCE(iocqMetrics.head_ptr);
    ProcessCE::Validate(ce, CESTAT_SUCCESS);  // throws upon error

    if (ce.n.SQID != IOQ_ID) {
        iocq->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "iocq", "CE.SQID"),
            "CE SQ ID Inconsistent");
        throw FrmwkEx(HERE, "Expected CE.SQID = 0x%04X in IOCQ CE but actual "
            "CE.SQID  = 0x%04X", IOQ_ID, ce.n.SQID);
    }

    if (ce.n.SQHD != expectedVal) {
        iocq->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "iocq", "CE.SQHD"),
            "CE SQ Head Pointer Inconsistent");
        throw FrmwkEx(HERE, "Expected CE.SQHD = 0x%04X in IOCQ CE but actual "
            "CE.SQHD  = 0x%04X", expectedVal, ce.n.SQHD);
    }
}


void
IOQRollChkDiff_r10b::VerifyQPointers(SharedIOSQPtr iosq, SharedIOCQPtr iocq)
{
    struct nvme_gen_cq iocqMetrics = iocq->GetQMetrics();
    struct nvme_gen_sq iosqMetrics = iosq->GetQMetrics();

    uint32_t expectedVal = (2 + MAX(iocq->GetNumEntries(),
        iosq->GetNumEntries())) % iocq->GetNumEntries();
    if (iocqMetrics.head_ptr != expectedVal) {
        iocq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "iocq",
            "head_ptr"), "CQ Metrics Head Pointer Inconsistent");
        throw FrmwkEx(HERE, "Expected IO CQ.head_ptr = 0x%04X but actual "
            "IOCQ.head_ptr = 0x%04X", expectedVal, iocqMetrics.head_ptr);
    }

    expectedVal = (2 + MAX(iocq->GetNumEntries(), iosq->GetNumEntries())) %
        iosq->GetNumEntries();
    if (iosqMetrics.tail_ptr != expectedVal) {
        iosq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "iosq",
            "tail_ptr"), "SQ Metrics Tail Pointer Inconsistent");
        throw FrmwkEx(HERE, "Expected  IO SQ.tail_ptr = 0x%04X but actual "
            "IOSQ.tail_ptr  = 0x%04X", expectedVal, iosqMetrics.tail_ptr);
    }
}


}   // namespace
