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

#include <boost/format.hpp>
#include "startingLBABare_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Cmds/write.h"
#include "../Utils/io.h"


namespace GrpNVMWriteReadCombo {


StartingLBABare_r10b::StartingLBABare_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Vary starting LBA for bare namspcs.");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "For all bare namspcs from Identify.NN, determine Identify.NCAP; For "
        "each namspc issue multiple write cmds each sending Identify.MDTS, or "
        "256 KB if unlimited, amount of data starting from LBA 0 to "
        "(Identify.NCAP - 1). Each block of data should use a new data pattern "
        "by rolling through {byte++, byteK, word++, wordK, dword++, dwordK}. "
        "After all writing completes issue correlating read cmds through the "
        "same range verifying the data pattern upon each block.");
}


StartingLBABare_r10b::~StartingLBABare_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


StartingLBABare_r10b::
StartingLBABare_r10b(const StartingLBABare_r10b &other) :
    Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


StartingLBABare_r10b &
StartingLBABare_r10b::operator=(const StartingLBABare_r10b
    &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
StartingLBABare_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
StartingLBABare_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */
    string work;
    bool enableLog;
    ConstSharedIdentifyPtr namSpcPtr;

    // Lookup objs which were created in a prior test within group
    SharedIOSQPtr iosq = CAST_TO_IOSQ(gRsrcMngr->GetObj(IOSQ_GROUP_ID));
    SharedIOCQPtr iocq = CAST_TO_IOCQ(gRsrcMngr->GetObj(IOCQ_GROUP_ID));

    ConstSharedIdentifyPtr idCmdCtrlr = gInformative->GetIdentifyCmdCtrlr();
    uint32_t maxDtXferSz = idCmdCtrlr->GetMaxDataXferSize();
    if (maxDtXferSz == 0)
        maxDtXferSz = MAX_DATA_TX_SIZE;

    LOG_NRM("Prepare cmds to be send through Q's.");
    SharedWritePtr writeCmd = SharedWritePtr(new Write());
    SharedMemBufferPtr writeMem = SharedMemBufferPtr(new MemBuffer());
    send_64b_bitmask prpBitmask = (send_64b_bitmask)
        (MASK_PRP1_PAGE | MASK_PRP2_PAGE | MASK_PRP2_LIST);

    SharedReadPtr readCmd = SharedReadPtr(new Read());
    SharedMemBufferPtr readMem = SharedMemBufferPtr(new MemBuffer());

    DataPattern dataPat[] = {
        DATAPAT_INC_8BIT,
        DATAPAT_CONST_8BIT,
        DATAPAT_INC_16BIT,
        DATAPAT_CONST_16BIT,
        DATAPAT_INC_32BIT,
        DATAPAT_CONST_32BIT
    };
    uint64_t dpArrSize = sizeof(dataPat) / sizeof(dataPat[0]);

    LOG_NRM("Seeking all bare namspc's.");
    vector<uint32_t> bare = gInformative->GetBareNamespaces();
    for (size_t i = 0; i < bare.size(); i++) {
        LOG_NRM("Processing for BARE name space id #%d", bare[i]);
        namSpcPtr = gInformative->GetIdentifyCmdNamspc(bare[i]);
        uint64_t ncap = namSpcPtr->GetValue(IDNAMESPC_NCAP);
        uint64_t lbaDataSize = namSpcPtr->GetLBADataSize();
        uint64_t maxWrBlks = maxDtXferSz / lbaDataSize;

        writeMem->Init(maxWrBlks * lbaDataSize);
        writeCmd->SetPrpBuffer(prpBitmask, writeMem);
        writeCmd->SetNSID(bare[i]);
        writeCmd->SetNLB(maxWrBlks - 1);  // 0 based value.

        readMem->Init(maxWrBlks * lbaDataSize);
        readCmd->SetPrpBuffer(prpBitmask, readMem);
        readCmd->SetNSID(bare[i]);
        readCmd->SetNLB(maxWrBlks - 1);  // 0 based value.

        for (uint64_t nWrBlks = 0; (nWrBlks + maxWrBlks - 1) < ncap; nWrBlks += maxWrBlks) {
            LOG_NRM("Sending #%ld blks of #%ld", nWrBlks, (ncap -1));
            if ((nWrBlks + maxWrBlks) >= ncap) {
                maxWrBlks = ncap - nWrBlks;
                LOG_NRM("Resize max write blocks to #%ld", maxWrBlks);
                ResizeDataBuf(readCmd, writeCmd, namSpcPtr, maxWrBlks,
                    prpBitmask);
            }            
            for (uint64_t nLBA = 0; nLBA < maxWrBlks; nLBA++) {
                writeMem->SetDataPattern(dataPat[nLBA % dpArrSize],
                    (nWrBlks + nLBA + 1), (nLBA * lbaDataSize), lbaDataSize);
            }
            writeCmd->SetSLBA(nWrBlks);
            readCmd->SetSLBA(nWrBlks);

            enableLog = false;
            if ((nWrBlks <= maxWrBlks) || (nWrBlks >= (ncap - 2 * maxWrBlks)))
                enableLog = true;
            work = str(boost::format("NSID.%d.SLBA.%ld") % bare[i] % nWrBlks);

            IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq,
                iocq, writeCmd, work, enableLog);

            IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq,
                iocq, readCmd, work, enableLog);

            VerifyDataPat(readCmd, writeMem);
        }
    }
}


void
StartingLBABare_r10b::VerifyDataPat(SharedReadPtr readCmd,
    SharedMemBufferPtr wrPayload)
{
    LOG_NRM("Compare read vs written data to verify");
    SharedMemBufferPtr rdPayload = readCmd->GetRWPrpBuffer();
    if (rdPayload->Compare(wrPayload) == false) {
        readCmd->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "ReadCmd"),
            "Read command");
        rdPayload->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "ReadPayload"),
            "Data read from media miscompared from written");
        wrPayload->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "WrittenPayload"),
            "Data read from media miscompared from written");
        throw FrmwkEx(HERE, "Data miscompare");
    }
}

void
StartingLBABare_r10b::ResizeDataBuf(SharedReadPtr &readCmd,
    SharedWritePtr &writeCmd, ConstSharedIdentifyPtr namSpcPtr,
    uint64_t maxWrBlks, send_64b_bitmask prpBitmask)
{
    LBAFormat lbaFormat = namSpcPtr->GetLBAFormat();
    uint64_t lbaDataSize = (1 << lbaFormat.LBADS);

    SharedMemBufferPtr readMem = readCmd->GetRWPrpBuffer();
    SharedMemBufferPtr writeMem = writeCmd->GetRWPrpBuffer();

    switch (gInformative->IdentifyNamespace(namSpcPtr)) {
    case Informative::NS_BARE:
    case Informative::NS_METAS:
        LOG_NRM("Resized max rd/wr blks to %ld for separate meta or bare", maxWrBlks);
        writeMem->Init(maxWrBlks * lbaDataSize);
        readMem->Init(maxWrBlks * lbaDataSize);
        break;
    case Informative::NS_METAI:
        LOG_NRM("Resized max rd/wr blks to %ld for integrated meta", maxWrBlks);
        writeMem->Init(maxWrBlks * (lbaDataSize + lbaFormat.MS));
        readMem->Init(maxWrBlks * (lbaDataSize + lbaFormat.MS));
        break;
    case Informative::NS_E2ES:
    case Informative::NS_E2EI:
    throw FrmwkEx(HERE, "Deferring work to handle this case in future");
    break;
    }

    writeCmd->SetPrpBuffer(prpBitmask, writeMem);
    writeCmd->SetNLB(maxWrBlks - 1); // 0 based value.

    readCmd->SetPrpBuffer(prpBitmask, readMem);
    readCmd->SetNLB(maxWrBlks - 1); // 0 based value.
}



}   // namespace
