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

#include "ioqRollChkSame_r10b.h"
#include "grpDefs.h"
#include "../Utils/io.h"
#include "../Utils/kernelAPI.h"

namespace GrpQueues {


IOQRollChkSame_r10b::IOQRollChkSame_r10b(int fd, string grpName,
    string testName, ErrorRegs errRegs) :
    Test(fd, grpName, testName, SPECREV_10b, errRegs)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4");
    mTestDesc.SetShort(     "Validate IOQ doorbell rollover when IOQ's same size");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Search for 1 of the following namspcs to run test. Find 1st bare "
        "namspc, or find 1st meta namspc, or find 1st E2E namspc. Create an "
        "IOSQ/IOCQ pair of size 2 and CAP.MQES; the pairs are the same size. "
        "Issue (max Q size plus 2) generic NVM write cmds, sending 1 block "
        "and approp supporting meta/E2E if necessary to the selected namspc "
        "at LBA 0, to fill and rollover the Q's, reaping each cmd as one is "
        "submitted, verify each CE.SQID is correct while filling. At the end "
        "of reaping all CE's verify IOSQ tail_ptr = 2, IOCQ head_ptr = 2, "
        "and CE.SQHD = 2.");
}


IOQRollChkSame_r10b::~IOQRollChkSame_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


IOQRollChkSame_r10b::
IOQRollChkSame_r10b(const IOQRollChkSame_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


IOQRollChkSame_r10b &
IOQRollChkSame_r10b::operator=(const IOQRollChkSame_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


void
IOQRollChkSame_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     *  \endverbatim
     */

    uint64_t maxIOQEntries;
    // Determine the max IOQ entries supported
    if (gRegisters->Read(CTLSPC_CAP, maxIOQEntries) == false)
        throw FrmwkEx("Unable to determine MQES");
    maxIOQEntries &= CAP_MQES;

    // IO Q Min Sizes
    IOQRollChkSame(2);
    // IO Q Max Sizes
    IOQRollChkSame((uint16_t)maxIOQEntries);
}

void
IOQRollChkSame_r10b::IOQRollChkSame(uint16_t numEntriesIOQ)
{
    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    if (gCtrlrConfig->SetState(ST_DISABLE) == false)
        throw FrmwkEx();

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx();

    uint8_t iocqes = (gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_CQES) & 0xf);
    gCtrlrConfig->SetIOCQES(iocqes);
    SharedMemBufferPtr iocqBackedMem = SharedMemBufferPtr(new MemBuffer());
    iocqBackedMem->InitOffset1stPage((numEntriesIOQ * (1 << iocqes)), 0, true);
    SharedIOCQPtr iocqContig = Queues::CreateIOCQDiscontigToHdw(mGrpName,
        mTestName, DEFAULT_CMD_WAIT_ms, asq, acq, IOQ_ID, numEntriesIOQ,
        false, IOCQ_CONTIG_GROUP_ID, false, 1, iocqBackedMem);

    uint8_t iosqes = (gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_SQES) & 0xf);
    gCtrlrConfig->SetIOSQES(iosqes);
    SharedMemBufferPtr iosqBackedMem = SharedMemBufferPtr(new MemBuffer());
    iosqBackedMem->InitOffset1stPage((numEntriesIOQ * (1 << iosqes)), 0, true);
    SharedIOSQPtr iosqContig = Queues::CreateIOSQDiscontigToHdw(mGrpName,
        mTestName, DEFAULT_CMD_WAIT_ms, asq, acq, IOQ_ID, numEntriesIOQ,
        false, IOSQ_CONTIG_GROUP_ID, IOQ_ID, 0, iosqBackedMem);

    LOG_NRM("(IOCQ Size, IOSQ Size)=(%d,%d)", iocqContig->GetNumEntries(),
        iosqContig->GetNumEntries());

    SharedWritePtr writeCmd = SetWriteCmd();

    LOG_NRM("Send #%d cmds to hdw via the contiguous IOSQ %d",
        iocqContig->GetNumEntries() + 2, iosqContig->GetQId());
    for (uint32_t numEntries = 0; numEntries <
        (uint32_t)(iosqContig->GetNumEntries() + 2); numEntries++) {
        iosqContig->Send(writeCmd);
        iosqContig->Ring();
        ReapAndVerifyCE(iocqContig, (numEntries + 1) %
            iosqContig->GetNumEntries());
    }
    VerifyQPointers(iosqContig, iocqContig);
}


SharedWritePtr
IOQRollChkSame_r10b::SetWriteCmd()
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
        prpBitmask = (send_64b_bitmask)(prpBitmask | MASK_MPTR);
    } else if (namspcData.type == Informative::NS_E2E) {
        writeCmd->AllocMetaBuffer();
        prpBitmask = (send_64b_bitmask)(prpBitmask | MASK_MPTR);
        LOG_ERR("Deferring E2E namspc work to the future");
        throw FrmwkEx("Need to add CRC's to correlate to buf pattern");
    }

    writeCmd->SetPrpBuffer(prpBitmask, dataPat);
    writeCmd->SetNSID(namspcData.id);
    writeCmd->SetNLB(0);

    return writeCmd;
}


void
IOQRollChkSame_r10b::ReapAndVerifyCE(SharedIOCQPtr iocq, uint16_t expectedVal)
{
    uint16_t numCE;
    uint16_t ceRemain;
    uint16_t numReaped;
    uint32_t isrCount;

    LOG_NRM("Wait for the CE to arrive in IOCQ %d", iocq->GetQId());
    if (iocq->ReapInquiryWaitSpecify(DEFAULT_CMD_WAIT_ms, 1, numCE, isrCount)
        == false) {

        iocq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "iocq",
            "reapInq"),
            "Unable to see any CE's in IOCQ, dump entire CQ contents");
        throw FrmwkEx("Unable to see completion of cmd");
    } else if (numCE != 1) {
        throw FrmwkEx("The IOCQ should only have 1 CE as a result of a cmd");
    }

    LOG_NRM("The CQ's metrics before reaping holds head_ptr");
    struct nvme_gen_cq iocqMetrics = iocq->GetQMetrics();

    LOG_NRM("Reaping CE from IOCQ, requires memory to hold reaped CE");
    SharedMemBufferPtr ceMemIOCQ = SharedMemBufferPtr(new MemBuffer());
    if ((numReaped = iocq->Reap(ceRemain, ceMemIOCQ, isrCount, numCE, true))
        != 1) {

        throw FrmwkEx("Verified there was 1 CE, but reaping produced %d",
            numReaped);
    }

    union CE ce = iocq->PeekCE(iocqMetrics.head_ptr);
    ProcessCE::Validate(ce, CESTAT_SUCCESS);  // throws upon error

    if (ce.n.SQID != IOQ_ID) {
        iocq->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "iocq", "CE.SQID"),
            "CE SQ ID Inconsistent");
        throw FrmwkEx("Expected CE.SQID = 0x%04X in IOCQ CE but actual "
            "CE.SQID  = 0x%04X", IOQ_ID, ce.n.SQID);
    }

    if (ce.n.SQHD != expectedVal) {
        iocq->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "iocq", "CE.SQHD"),
            "CE SQ Head Pointer Inconsistent");
        throw FrmwkEx("Expected CE.SQHD = 0x%04X in IOCQ CE but actual "
            "CE.SQHD  = 0x%04X", expectedVal, ce.n.SQHD);
    }
}


void
IOQRollChkSame_r10b::VerifyQPointers(SharedIOSQPtr iosq, SharedIOCQPtr iocq)
{
    uint16_t expectedVal = 2;
    struct nvme_gen_cq iocqMetrics = iocq->GetQMetrics();
    struct nvme_gen_sq iosqMetrics = iosq->GetQMetrics();

    if (iosqMetrics.tail_ptr == 0)
        expectedVal = 0;
    if (iosqMetrics.tail_ptr != expectedVal) {
        iosq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "iosq",
            "tail_ptr"), "SQ Metrics Tail Pointer Inconsistent");
        throw FrmwkEx("Expected  IO SQ.tail_ptr = 0x%04X but actual "
            "IOSQ.tail_ptr  = 0x%04X", expectedVal, iosqMetrics.tail_ptr);
    }

    expectedVal = 2;
    if (iocqMetrics.head_ptr == 0)
        expectedVal = 0;
    if (iocqMetrics.head_ptr != expectedVal) {
        iocq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "iocq",
            "head_ptr"), "CQ Metrics Head Pointer Inconsistent");
        throw FrmwkEx("Expected IO CQ.head_ptr = 0x%04X but actual "
            "IOCQ.head_ptr = 0x%04X", expectedVal, iocqMetrics.head_ptr);
    }
}


}   // namespace
