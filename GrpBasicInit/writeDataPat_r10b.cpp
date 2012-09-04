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

#include "writeDataPat_r10b.h"
#include "globals.h"
#include "createIOQContigPoll_r10b.h"
#include "createIOQDiscontigPoll_r10b.h"
#include "grpDefs.h"
#include "../Utils/kernelAPI.h"
#include "../Queues/ce.h"

namespace GrpBasicInit {


WriteDataPat_r10b::WriteDataPat_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Write a well known data pattern to media");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Search for 1 of the following namspcs to run test. Find 1st bare "
        "namspc, or find 1st meta namspc, or find 1st E2E namspc. Issue "
        "identical write cmd to the selected namspc starting at LBA 0, "
        "sending a single block with approp meta/E2E requirements if "
        "necessary. Issue an NVM cmd set write command with a well known "
        "data pattern to namespace found. The write command shall be "
        "completely generic.");
}


WriteDataPat_r10b::~WriteDataPat_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


WriteDataPat_r10b::
WriteDataPat_r10b(const WriteDataPat_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


WriteDataPat_r10b &
WriteDataPat_r10b::operator=(const WriteDataPat_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
WriteDataPat_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
WriteDataPat_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Both test CreateIOQContigPoll_r10b & CreateIOQDiscontigPoll_r10b has
     *    run prior, or Both test CreateIOQContigIrq_r10b &
     *    CreateIOQDiscontigIrq_r10b has run prior
     * 2) An individual test within this group cannot run, the entire group
     *    must be executed every time. Each subsequent test relies on the prior.
     * \endverbatim
     */

    WriteDataPattern();
}


void
WriteDataPat_r10b::WriteDataPattern()
{
    uint64_t regVal;

    LOG_NRM("Calc buffer size to write %d logical blks to media",
        WRITE_DATA_PAT_NUM_BLKS);
    Informative::Namspc namspcData = gInformative->Get1stBareMetaE2E();
    ConstSharedIdentifyPtr namSpcPtr = namspcData.idCmdNamspc;
    if (namSpcPtr == Identify::NullIdentifyPtr)
        throw FrmwkEx(HERE, "Namespace #%d must exist", namspcData.id);

    LBAFormat lbaFormat = namspcData.idCmdNamspc->GetLBAFormat();
    if (namspcData.type == Informative::NS_METAS) {
        if (gRsrcMngr->SetMetaAllocSize(lbaFormat.MS * WRITE_DATA_PAT_NUM_BLKS)
            == false) {
            throw FrmwkEx(HERE);
        }
    }
    uint64_t lbaDataSize = namSpcPtr->GetLBADataSize();

    LOG_NRM("Create a generic write cmd to send data pattern to namspc #%d",
        namspcData.id);
    SharedWritePtr writeCmd = SharedWritePtr(new Write());
    send_64b_bitmask prpBitmask = (send_64b_bitmask)
        (MASK_PRP1_PAGE | MASK_PRP2_PAGE | MASK_PRP2_LIST);

    LOG_NRM("Create data pattern to write to media");
    SharedMemBufferPtr dataPat = SharedMemBufferPtr(new MemBuffer());

    switch (namspcData.type) {
    case Informative::NS_BARE:
        dataPat->Init(WRITE_DATA_PAT_NUM_BLKS * lbaDataSize);
        break;
    case Informative::NS_METAS:
        dataPat->Init(WRITE_DATA_PAT_NUM_BLKS * lbaDataSize);
        writeCmd->AllocMetaBuffer();
        writeCmd->SetMetaDataPattern(DATAPAT_INC_16BIT);
        break;
    case Informative::NS_METAI:
        dataPat->Init(WRITE_DATA_PAT_NUM_BLKS * (lbaDataSize + lbaFormat.MS));
        break;
    case Informative::NS_E2ES:
    case Informative::NS_E2EI:
        throw FrmwkEx(HERE, "Deferring work to handle this case in future");
        break;
    }

    dataPat->SetDataPattern(DATAPAT_INC_16BIT);
    dataPat->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "DataPat"),
        "Write buffer's data pattern");

    writeCmd->SetPrpBuffer(prpBitmask, dataPat);
    writeCmd->SetNSID(namspcData.id);
    writeCmd->SetNLB(WRITE_DATA_PAT_NUM_BLKS - 1);  // convert to 0-based value

    // Lookup objs which were created in a prior test within group
    SharedIOSQPtr iosqContig = CAST_TO_IOSQ(
        gRsrcMngr->GetObj(IOSQ_CONTIG_GROUP_ID))
    SharedIOCQPtr iocqContig = CAST_TO_IOCQ(
        gRsrcMngr->GetObj(IOCQ_CONTIG_GROUP_ID))
    SharedIOSQPtr iosqDiscontig = CAST_TO_IOSQ(
        gRsrcMngr->GetObj(IOSQ_DISCONTIG_GROUP_ID))
    SharedIOCQPtr iocqDiscontig = CAST_TO_IOCQ(
        gRsrcMngr->GetObj(IOCQ_DISCONTIG_GROUP_ID))

    LOG_NRM("Send the cmd to hdw via the contiguous IOQ's");
    SendToIOSQ(iosqContig, iocqContig, writeCmd, "contig");

    // To run the discontig part of this test, the hdw must support that feature
    if (gRegisters->Read(CTLSPC_CAP, regVal) == false) {
        throw FrmwkEx(HERE, "Unable to determine Q memory requirements");
    } else if (regVal & CAP_CQR) {
        LOG_NRM("Unable to utilize discontig Q's, DUT requires contig");
        return;
    }

    LOG_NRM("Send the cmd to hdw via the discontiguous IOQ's");
    SendToIOSQ(iosqDiscontig, iocqDiscontig, writeCmd, "discontig");
}


void
WriteDataPat_r10b::SendToIOSQ(SharedIOSQPtr iosq, SharedIOCQPtr iocq,
    SharedWritePtr writeCmd, string qualifier)
{
    uint32_t numCE;
    uint32_t ceRemain;
    uint32_t numReaped;
    uint32_t isrCount;
    uint16_t uniqueId;


    LOG_NRM("Send the cmd to hdw via %s IOSQ", qualifier.c_str());
    iosq->Send(writeCmd, uniqueId);
    iosq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "iosq", qualifier),
        "Just B4 ringing SQ doorbell, dump entire IOSQ contents");
    iosq->Ring();


    LOG_NRM("Wait for the CE to arrive in IOCQ");
    if (iocq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(1), 1, numCE, isrCount)
        == false) {

        iocq->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "iocq", qualifier),
            "Unable to see any CE's in IOCQ, dump entire CQ contents");
        throw FrmwkEx(HERE, "Unable to see completion of cmd");
    } else if (numCE != 1) {
        iocq->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "iocq", qualifier),
            "Unable to see any CE's in IOCQ, dump entire CQ contents");
        throw FrmwkEx(HERE,
            "The IOCQ should only have 1 CE as a result of a cmd");
    }
    iocq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "iocq", qualifier),
        "Just B4 reaping IOCQ, dump entire CQ contents");


    LOG_NRM("The CQ's metrics B4 reaping holds head_ptr needed");
    struct nvme_gen_cq iocqMetrics = iocq->GetQMetrics();
    KernelAPI::LogCQMetrics(iocqMetrics);

    LOG_NRM("Reaping CE from IOCQ, requires memory to hold reaped CE");
    SharedMemBufferPtr ceMemIOCQ = SharedMemBufferPtr(new MemBuffer());
    if ((numReaped = iocq->Reap(ceRemain, ceMemIOCQ, isrCount, numCE, true))
        != 1) {

        throw FrmwkEx(HERE, "Verified there was 1 CE, but reaping produced %d",
            numReaped);
    }
    LOG_NRM("The reaped CE is...");
    iocq->LogCE(iocqMetrics.head_ptr);

    union CE ce = iocq->PeekCE(iocqMetrics.head_ptr);
    ProcessCE::Validate(ce);  // throws upon error
}

}   // namespace
