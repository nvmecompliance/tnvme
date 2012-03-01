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

#define WRITE_DATA_PAT_NUM_BLKS     1


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


bool
IOQRollChkSame_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     *  \endverbatim
     */
    uint64_t maxIOQEntries;

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    if (gInformative->GetFeaturesNumOfIOCQs() < IOQ_ID) {
        LOG_ERR("DUT doesn't support %d IOCQ's", IOQ_ID);
        throw exception();
    } else if (gInformative->GetFeaturesNumOfIOSQs() < IOQ_ID) {
        LOG_ERR("DUT doesn't support %d IOSQ's", IOQ_ID);
        throw exception();
    }

    // Determine the max IOQ entries supported
    if (gRegisters->Read(CTLSPC_CAP, maxIOQEntries) == false) {
        LOG_ERR("Unable to determine MQES");
        throw exception();
    }
    maxIOQEntries &= CAP_MQES;

    // IO Q Min Sizes
    DisableAndEnableCtrl();
    IOQRollChkSame(asq, acq, 2);

    // IO Q Max Sizes
    DisableAndEnableCtrl();
    IOQRollChkSame(asq, acq, (uint16_t)maxIOQEntries);

    return true;
}

void
IOQRollChkSame_r10b::IOQRollChkSame(SharedASQPtr asq, SharedACQPtr acq,
    uint16_t numEntriesIOQ)
{
    SharedWritePtr writeCmd;


    gCtrlrConfig->SetIOCQES(IOCQ::COMMON_ELEMENT_SIZE_PWR_OF_2);
    SharedIOCQPtr iocqContig = Queues::CreateIOCQContigToHdw(mFd, mGrpName,
        mTestName, DEFAULT_CMD_WAIT_ms, asq, acq, IOQ_ID, numEntriesIOQ,
        false, IOCQ_CONTIG_GROUP_ID, false, 1);

    gCtrlrConfig->SetIOSQES(IOSQ::COMMON_ELEMENT_SIZE_PWR_OF_2);
    SharedIOSQPtr iosqContig = Queues::CreateIOSQContigToHdw(mFd, mGrpName,
        mTestName, DEFAULT_CMD_WAIT_ms, asq, acq, IOQ_ID, numEntriesIOQ,
        false, IOSQ_CONTIG_GROUP_ID, IOQ_ID, 0);

    LOG_NRM("(IOCQ Size, IOSQ Size)=(%d,%d)", iocqContig->GetNumEntries(),
        iosqContig->GetNumEntries());

    ConstSharedIdentifyPtr namspc = gInformative->Get1stBareMetaE2E();
    Informative::NamspcType nsType = gInformative->IdentifyNamespace(namspc);
    SetWriteCmd(namspc, writeCmd, nsType);

    LOG_NRM("Send #%d cmds to hdw via the contiguous IOQ's",
        iocqContig->GetNumEntries() + 2);
    for (uint32_t numEntries = 0; numEntries <
        (uint32_t)(iosqContig->GetNumEntries() + 2); numEntries++) {
        SendToIOSQ(iosqContig, iocqContig, writeCmd, "contig");
        VerifyCESQValues(iocqContig, (numEntries + 1) %
            iosqContig->GetNumEntries());
    }
    VerifyQPointers(iosqContig, iocqContig);
}


void
IOQRollChkSame_r10b::SetWriteCmd(ConstSharedIdentifyPtr& namSpcPtr,
    SharedWritePtr& writeCmd, Informative::NamspcType nsType)
{
    LOG_NRM("Processing write cmd using Bare namspc # 1");
    uint64_t lbaDataSize = namSpcPtr->GetLBADataSize();

    LOG_NRM("Create data pattern to write to media");
    SharedMemBufferPtr dataPat = SharedMemBufferPtr(new MemBuffer());
    dataPat->Init(WRITE_DATA_PAT_NUM_BLKS * lbaDataSize);
    dataPat->SetDataPattern(MemBuffer::DATAPAT_INC_16BIT);

    LOG_NRM("Create generic write cmd to send data pattern to bare namspc 1");
    writeCmd = SharedWritePtr(new Write(mFd));
    send_64b_bitmask prpBitmask = (send_64b_bitmask)(MASK_PRP1_PAGE
        | MASK_PRP2_PAGE | MASK_PRP2_LIST);
    writeCmd->SetPrpBuffer(prpBitmask, dataPat);
    writeCmd->SetNSID(1);
    writeCmd->SetNLB(WRITE_DATA_PAT_NUM_BLKS - 1); // convert to 0-based value

    if (nsType == Informative::NS_META) {
        writeCmd->AllocMetaBuffer();
    } else if (nsType == Informative::NS_E2E) {
        writeCmd->AllocMetaBuffer();
        LOG_ERR("Deferring E2E namspc work to the future");
        LOG_ERR("Need to add CRC's to correlate to buf pattern");
        throw exception();
    }
}


void
IOQRollChkSame_r10b::SetMetaDataSize()
{
    ConstSharedIdentifyPtr namSpcPtr;
    vector<uint32_t> nameSpc = gInformative->GetMetaNamespaces();
    if (nameSpc.size()) {
        namSpcPtr = gInformative->GetIdentifyCmdNamspc(nameSpc[0]);
        if (namSpcPtr == Identify::NullIdentifyPtr)
            throw exception();
        LBAFormat lbaFormat = namSpcPtr->GetLBAFormat();
        if (gRsrcMngr->SetMetaAllocSize(lbaFormat.MS) == false)
            throw exception();
    }
}


void
IOQRollChkSame_r10b::DisableAndEnableCtrl()
{
    if (gCtrlrConfig->SetState(ST_DISABLE) == false)
        throw exception();

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw exception();

    SetMetaDataSize();
}


void
IOQRollChkSame_r10b::SendToIOSQ(SharedIOSQPtr iosq, SharedIOCQPtr iocq,
    SharedWritePtr writeCmd, string qualifier)
{
    uint16_t numCE;
    uint16_t ceRemain;
    uint16_t numReaped;
    uint32_t isrCount;

    LOG_NRM("Send the cmd to hdw via %s IOSQ", qualifier.c_str());
    iosq->Send(writeCmd);
    iosq->Ring();

    LOG_NRM("Wait for the CE to arrive in IOCQ");
    if (iocq->ReapInquiryWaitSpecify(DEFAULT_CMD_WAIT_ms, 1, numCE, isrCount)
        == false) {
        LOG_ERR("Unable to see completion of cmd");
        iocq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "iocq",
            qualifier),
            "Unable to see any CE's in IOCQ, dump entire CQ contents");
        throw exception();
    } else if (numCE != 1) {
        LOG_ERR("The IOCQ should only have 1 CE as a result of a cmd");
        throw exception();
    }

    LOG_NRM("Reaping CE from IOCQ, requires memory to hold reaped CE");
    SharedMemBufferPtr ceMemIOCQ = SharedMemBufferPtr(new MemBuffer());
    if ((numReaped = iocq->Reap(ceRemain, ceMemIOCQ, isrCount, numCE, true))
        != 1) {
        LOG_ERR("Verified there was 1 CE, but reaping produced %d", numReaped);
        throw exception();
    }
}


void
IOQRollChkSame_r10b::VerifyCESQValues(SharedIOCQPtr iocq,
    uint16_t expectedVal)
{
    union CE ce;
    struct nvme_gen_cq iocqMetrics = iocq->GetQMetrics();

    // The CQ's metrics after reaping holds head_ptr plus 1 needed. Also Take
    // Q roll over into account
    if (iocqMetrics.head_ptr == 0) {
        ce = iocq->PeekCE(iocq->GetNumEntries() - 1);
    } else {
        ce = iocq->PeekCE(iocqMetrics.head_ptr - 1);
    }

    if (ce.n.SQID != IOQ_ID) {
        LOG_ERR("Expected CE.SQID = 0x%04X in IOCQ CE but actual "
            "CE.SQID  = 0x%04X", IOQ_ID, ce.n.SQID);
        iocq->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "iocq", "CE.SQID"),
            "CE SQ ID Inconsistent");
        throw exception();
    }

    if (ce.n.SQHD != expectedVal) {
        LOG_ERR("Expected CE.SQHD = 0x%04X in IOCQ CE but actual "
            "CE.SQHD  = 0x%04X", expectedVal, ce.n.SQHD);
        iocq->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "iocq", "CE.SQHD"),
            "CE SQ Head Pointer Inconsistent");
        throw exception();
    }
}

void
IOQRollChkSame_r10b::VerifyQPointers(SharedIOSQPtr iosq, SharedIOCQPtr iocq)
{
    uint16_t expectedVal = 2;
    union CE ce;
    struct nvme_gen_cq iocqMetrics = iocq->GetQMetrics();
    struct nvme_gen_sq iosqMetrics = iosq->GetQMetrics();

    // The CQ's metrics after reaping holds head_ptr plus 1 needed. Also Take
    // Q roll over into account.
    if (iocqMetrics.head_ptr == 0) {
        ce = iocq->PeekCE(iocq->GetNumEntries() - 1);
        expectedVal = 0;
    } else {
        ce = iocq->PeekCE(iocqMetrics.head_ptr - 1);
    }

    if (iosqMetrics.tail_ptr != expectedVal) {
        LOG_ERR("Expected  IO SQ.tail_ptr = 0x%04X but actual "
            "IOSQ.tail_ptr  = 0x%04X", expectedVal, iosqMetrics.tail_ptr);
        iosq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "iosq",
            "tail_ptr"), "SQ Metrics Tail Pointer Inconsistent");
        throw exception();
    }

    if (iocqMetrics.head_ptr != expectedVal) {
        LOG_ERR("Expected IO CQ.head_ptr = 0x%04X but actual "
            "IOCQ.head_ptr = 0x%04X", expectedVal, iocqMetrics.head_ptr);
        iocq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "iocq",
            "head_ptr"), "CQ Metrics Head Pointer Inconsistent");
        throw exception();
    }

    if (ce.n.SQHD != expectedVal) {
        LOG_ERR("Expected IO CE.SQHD = 0x%04X but actual CE.SQHD  = 0x%04X",
            expectedVal, ce.n.SQHD);
        iocq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "iocq",
            "CE.SQHD"), "CE SQ Head Pointer Inconsistent");
        throw exception();
    }
}

}   // namespace
