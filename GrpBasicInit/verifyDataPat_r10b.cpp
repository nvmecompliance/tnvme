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
#include "grpDefs.h"
#include "createIOQContigPoll_r10b.h"
#include "createIOQDiscontigPoll_r10b.h"
#include "writeDataPat_r10b.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/io.h"

namespace GrpBasicInit {


VerifyDataPat_r10b::VerifyDataPat_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Verify a well known data pattern from media");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Search for 1 of the following namspcs to run test. Find 1st bare "
        "namspc, or find 1st meta namspc, or find 1st E2E namspc. Issue an "
        "NVM cmd set read command with approp meta/E2E requirements if "
        "necessary and compare the data payload with a previously written "
        "and well known data pattern to the same namespace id. The "
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


Test::RunType
VerifyDataPat_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    preserve = preserve;    // Suppress compiler error/warning
    return RUN_TRUE;        // This test is never destructive
}


void
VerifyDataPat_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test WriteDataPat_r10b has run prior.
     * 2) An individual test within this group cannot run, the entire group
     *    must be executed every time. Each subsequent test relies on the prior.
     * \endverbatim
     */

    VerifyDataPattern();
}


void
VerifyDataPat_r10b::VerifyDataPattern()
{
    uint64_t regVal;


    LOG_NRM("Calc buffer size to read %d log blks from media",
        WRITE_DATA_PAT_NUM_BLKS);
    Informative::Namspc namspcData = gInformative->Get1stBareMetaE2E();
    ConstSharedIdentifyPtr namSpcPtr = namspcData.idCmdNamspc;
    if (namSpcPtr == Identify::NullIdentifyPtr)
        throw FrmwkEx(HERE, "Namespace #%d must exist", namspcData.id);
    uint64_t lbaDataSize = namSpcPtr->GetLBADataSize();
    LBAFormat lbaFormat = namspcData.idCmdNamspc->GetLBAFormat();

    LOG_NRM("Create data pattern to compare against");
    SharedMemBufferPtr dataPat = SharedMemBufferPtr(new MemBuffer());

    LOG_NRM("Create memory to contain read payload");
    SharedMemBufferPtr readMem = SharedMemBufferPtr(new MemBuffer());
    LOG_NRM("Create a generic read cmd to read data from namspc #%d",
        namspcData.id);
    SharedReadPtr readCmd = SharedReadPtr(new Read());

    switch (namspcData.type) {
    case Informative::NS_BARE:
        dataPat->Init(WRITE_DATA_PAT_NUM_BLKS * lbaDataSize);
        readMem->Init(WRITE_DATA_PAT_NUM_BLKS * lbaDataSize);
        break;
    case Informative::NS_METAS:
        dataPat->Init(WRITE_DATA_PAT_NUM_BLKS * lbaDataSize);
        readMem->Init(WRITE_DATA_PAT_NUM_BLKS * lbaDataSize);
        readCmd->AllocMetaBuffer();
        break;
    case Informative::NS_METAI:
        dataPat->Init(WRITE_DATA_PAT_NUM_BLKS * (lbaDataSize + lbaFormat.MS));
        readMem->Init(WRITE_DATA_PAT_NUM_BLKS * (lbaDataSize + lbaFormat.MS));
        break;
    case Informative::NS_E2ES:
    case Informative::NS_E2EI:
        throw FrmwkEx(HERE, "Deferring work to handle this case in future");
        break;
    }

    dataPat->SetDataPattern(DATAPAT_INC_16BIT);
    dataPat->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "DataPat"),
        "Verify buffer's data pattern");

    send_64b_bitmask prpBitmask = (send_64b_bitmask)
        (MASK_PRP1_PAGE | MASK_PRP2_PAGE | MASK_PRP2_LIST);
    readCmd->SetPrpBuffer(prpBitmask, readMem);
    readCmd->SetNSID(namspcData.id);
    readCmd->SetNLB(WRITE_DATA_PAT_NUM_BLKS - 1);    // convert to 0-based value

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
    SendToIOSQ(iosqContig, iocqContig, readCmd, "contig", dataPat, readMem);

    // To run the discontig part of this test, the hdw must support that feature
    if (gRegisters->Read(CTLSPC_CAP, regVal) == false) {
        throw FrmwkEx(HERE, "Unable to determine Q memory requirements");
    } else if (regVal & CAP_CQR) {
        LOG_NRM("Unable to utilize discontig Q's, DUT requires contig");
        return;
    }

    LOG_NRM("Send the cmd to hdw via the discontiguous IOQ's");
    SendToIOSQ(iosqDiscontig, iocqDiscontig, readCmd, "discontig", dataPat,
        readMem);
}


void
VerifyDataPat_r10b::SendToIOSQ(SharedIOSQPtr iosq, SharedIOCQPtr iocq,
    SharedReadPtr readCmd, string qualifier, SharedMemBufferPtr writtenPayload,
    SharedMemBufferPtr readPayload)
{
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq,
        readCmd, qualifier, true);

    LOG_NRM("Compare read vs written data to verify");
    if (readPayload->Compare(writtenPayload) == false) {
        readPayload->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "ReadPayload"),
            "Data read from media miscompared from written");
        writtenPayload->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "WrittenPayload"),
            "Data read from media miscompared from written");
        throw FrmwkEx(HERE, "Data miscompare");
    }
}

}   // namespace
