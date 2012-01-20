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

#include "verifyDataPat_r10b.h"
#include "globals.h"
#include "../Utils/kernelAPI.h"
#include "createIOQContigPoll_r10b.h"
#include "createIOQDiscontigPoll_r10b.h"
#include "writeDataPat_r10b.h"

#define DEFAULT_CMD_WAIT_ms         2000


VerifyDataPat_r10b::VerifyDataPat_r10b(int fd, string grpName, string testName,
    ErrorRegs errRegs) :
    Test(fd, grpName, testName, SPECREV_10b, errRegs)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Verify a well known data pattern from media");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Issue an NVM cmd set read command and compare the data payload with a "
        "previsouly written and well known data pattern from namespace #1. The "
        "read command shall be completely generic.");
}


VerifyDataPat_r10b::~VerifyDataPat_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


VerifyDataPat_r10b::
VerifyDataPat_r10b(const VerifyDataPat_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


VerifyDataPat_r10b &
VerifyDataPat_r10b::operator=(const VerifyDataPat_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


bool
VerifyDataPat_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) All interrupts are disabled.
     * 2) Contigous IOQ pairs have been created by the RsrcMngr for group life
     * 3) The NVM cmd set is the active cmd set.
     * \endverbatim
     */

    KernelAPI::DumpKernelMetrics(mFd,
        FileSystem::PrepLogFile(mGrpName, mTestName, "kmetrics", "before"));

    VerifyDataPattern();

    KernelAPI::DumpKernelMetrics(mFd,
        FileSystem::PrepLogFile(mGrpName, mTestName, "kmetrics", "after"));
    return true;
}


void
VerifyDataPat_r10b::VerifyDataPattern()
{
    LOG_NRM("Calc buffer size to read %d log blks from media",
        WRITE_DATA_PAT_NUM_BLKS);
    ConstSharedIdentifyPtr namSpcPtr = gInformative->GetIdentifyCmdNamespace(1);
    if (namSpcPtr == Identify::NullIdentifyPtr) {
        LOG_ERR("Namespace #1 must exist");
        throw exception();
    }
    uint64_t lbaDataSize = namSpcPtr->GetLBADataSize();


    LOG_NRM("Create data pattern to compare against");
    SharedMemBufferPtr dataPat = SharedMemBufferPtr(new MemBuffer());
    dataPat->Init(WRITE_DATA_PAT_NUM_BLKS * lbaDataSize);
    dataPat->SetDataPattern(MemBuffer::DATAPAT_INC_16BIT);
    dataPat->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "DataPat"),
        "Verify buffer's data pattern");
    
    LOG_NRM("Create memory to contain read payload");
    SharedMemBufferPtr readMem = SharedMemBufferPtr(new MemBuffer());
    readMem->Init(WRITE_DATA_PAT_NUM_BLKS * lbaDataSize);

    LOG_NRM("Create a generic read cmd to read data from namspc 1");
    SharedReadPtr readCmd = SharedReadPtr(new Read(mFd));
    send_64b_bitmask prpBitmask = (send_64b_bitmask)
        (MASK_PRP1_PAGE | MASK_PRP2_PAGE | MASK_PRP2_LIST);
    readCmd->SetPrpBuffer(prpBitmask, readMem);
    readCmd->SetNSID(1);
    readCmd->SetNLB(WRITE_DATA_PAT_NUM_BLKS - 1);    // convert to 0-based value

    // Lookup objs which were created in a prior test within group
    SharedIOSQPtr iosqContig = CAST_TO_IOSQ(
        gRsrcMngr->GetObj(IOSQ_CONTIG_POLL_GROUP_ID))
    SharedIOCQPtr iocqContig = CAST_TO_IOCQ(
        gRsrcMngr->GetObj(IOCQ_CONTIG_POLL_GROUP_ID))
    SharedIOSQPtr iosqDiscontig = CAST_TO_IOSQ(
        gRsrcMngr->GetObj(IOSQ_DISCONTIG_POLL_GROUP_ID))
    SharedIOCQPtr iocqDiscontig = CAST_TO_IOCQ(
        gRsrcMngr->GetObj(IOCQ_DISCONTIG_POLL_GROUP_ID))

    LOG_NRM("Send the cmd to hdw via the contiguous IOQ's");
    SendToIOSQ(iosqContig, iocqContig, readCmd, "contig", dataPat, readMem);
    LOG_NRM("Send the cmd to hdw via the discontiguous IOQ's");
    SendToIOSQ(iosqDiscontig, iocqDiscontig, readCmd, "discontig", dataPat,
        readMem);
}


void
VerifyDataPat_r10b::SendToIOSQ(SharedIOSQPtr iosq, SharedIOCQPtr iocq,
    SharedReadPtr readCmd, string qualifier, SharedMemBufferPtr writtenPayload,
    SharedMemBufferPtr readPayload)
{
    uint16_t numCE;
    uint16_t ceRemain;
    uint16_t numReaped;


    LOG_NRM("Send the cmd to hdw via %s IOSQ", qualifier.c_str());
    iosq->Send(readCmd);
    iosq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "iosq", qualifier),
        "Just B4 ringing SQ doorbell, dump entire IOSQ contents");
    iosq->Ring();


    LOG_NRM("Wait for the CE to arrive in IOCQ");
    if (iocq->ReapInquiryWaitSpecify(DEFAULT_CMD_WAIT_ms, 1, numCE) == false) {
        LOG_ERR("Unable to see completion of cmd");
        iocq->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "iocq", qualifier),
            "Unable to see any CE's in IOCQ, dump entire CQ contents");
        throw exception();
    } else if (numCE != 1) {
        LOG_ERR("The IOCQ should only have 1 CE as a result of a cmd");
        throw exception();
    }
    iocq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "iocq", qualifier),
        "Just B4 reaping IOCQ, dump entire CQ contents");


    LOG_NRM("The CQ's metrics B4 reaping holds head_ptr needed");
    struct nvme_gen_cq iocqMetrics = iocq->GetQMetrics();
    KernelAPI::LogCQMetrics(iocqMetrics);

    LOG_NRM("Reaping CE from IOCQ, requires memory to hold reaped CE");
    SharedMemBufferPtr ceMemIOCQ = SharedMemBufferPtr(new MemBuffer());
    if ((numReaped = iocq->Reap(ceRemain, ceMemIOCQ, numCE, true)) != 1) {
        LOG_ERR("Verified there was 1 CE, but reaping produced %d", numReaped);
        throw exception();
    }
    LOG_NRM("The reaped CE is...");
    iocq->LogCE(iocqMetrics.head_ptr);

    union CE ce = iocq->PeekCE(iocqMetrics.head_ptr);
    ProcessCE::ValidateStatus(ce);  // throws upon error


    LOG_NRM("Compare read vs written data to verify");
    if (readPayload->Compare(writtenPayload) == false) {
        readPayload->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "ReadPayload"),
            "Data read from media miscompared from written");
        writtenPayload->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "WrittenPayload"),
            "Data read from media miscompared from written");
    }
}
