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


VerifyDataPat_r10b::VerifyDataPat_r10b(int fd, string grpName, string testName,
    ErrorRegs errRegs) :
    Test(fd, grpName, testName, SPECREV_10b, errRegs)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
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
    ConstSharedIdentifyPtr namSpcPtr = gInformative->GetIdentifyCmdNamspc(1);
    if (namSpcPtr == Identify::NullIdentifyPtr)
        throw FrmwkEx("Namespace #1 must exist");
    uint64_t lbaDataSize = namSpcPtr->GetLBADataSize();


    LOG_NRM("Create data pattern to compare against");
    SharedMemBufferPtr dataPat = SharedMemBufferPtr(new MemBuffer());
    dataPat->Init(WRITE_DATA_PAT_NUM_BLKS * lbaDataSize);
    dataPat->SetDataPattern(DATAPAT_INC_16BIT);
    dataPat->Dump(FileSystem::PrepLogFile(mGrpName, mTestName, "DataPat"),
        "Verify buffer's data pattern");
    
    LOG_NRM("Create memory to contain read payload");
    SharedMemBufferPtr readMem = SharedMemBufferPtr(new MemBuffer());
    readMem->Init(WRITE_DATA_PAT_NUM_BLKS * lbaDataSize);

    LOG_NRM("Create a generic read cmd to read data from namspc 1");
    SharedReadPtr readCmd = SharedReadPtr(new Read());
    send_64b_bitmask prpBitmask = (send_64b_bitmask)
        (MASK_PRP1_PAGE | MASK_PRP2_PAGE | MASK_PRP2_LIST);
    readCmd->SetPrpBuffer(prpBitmask, readMem);
    readCmd->SetNSID(1);
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
        throw FrmwkEx("Unable to determine Q memory requirements");
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
    IO::SendCmdToHdw(mGrpName, mTestName, DEFAULT_CMD_WAIT_ms, iosq, iocq,
        readCmd, qualifier, true);

    LOG_NRM("Compare read vs written data to verify");
    if (readPayload->Compare(writtenPayload) == false) {
        readPayload->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "ReadPayload"),
            "Data read from media miscompared from written");
        writtenPayload->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "WrittenPayload"),
            "Data read from media miscompared from written");
        throw FrmwkEx("Data miscompare");
    }
}

}   // namespace
