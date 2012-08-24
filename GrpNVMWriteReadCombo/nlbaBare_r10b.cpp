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
#include "nlbaBare_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Cmds/write.h"
#include "../Utils/io.h"


namespace GrpNVMWriteReadCombo {

// Maximum number of bits for logical blks (NLB) in cmd DWORD 12 for rd/wr cmd.
#define CDW12_NLB_BITS          16


NLBABare_r10b::NLBABare_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Verify all values of Number of Log Blk (NLB) for bare namspcs");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "For all bare namspcs from Identify.NN; For each namspc issue "
        "consecutive write cmds each staring at LBA 0 by looping thru all "
        "values for DW12.NLB from 0 to {0xffff | (Identify.MDTS / "
        "Identify.LBAF[Identify.FLBAS].LBADS) | NCAP} "
        "which ever is less. Each write cmd should use a new data pattern by "
        "rolling through {byte++, byteK, word++, wordK, dword++, dwordK}. "
        "After each write cmd completes issue a correlating read cmd through "
        "the same parameters verifying the data pattern.");
}


NLBABare_r10b::~NLBABare_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


NLBABare_r10b::
NLBABare_r10b(const NLBABare_r10b &other) :
    Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


NLBABare_r10b &
NLBABare_r10b::operator=(const NLBABare_r10b
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
NLBABare_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
NLBABare_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */
    string work;
    bool enableLog;
    ConstSharedIdentifyPtr namSpcPtr;

    LOG_NRM("Lookup objs which were created in a prior test within group");
    SharedIOSQPtr iosq = CAST_TO_IOSQ(gRsrcMngr->GetObj(IOSQ_GROUP_ID));
    SharedIOCQPtr iocq = CAST_TO_IOCQ(gRsrcMngr->GetObj(IOCQ_GROUP_ID));

    ConstSharedIdentifyPtr idCmdCtrlr = gInformative->GetIdentifyCmdCtrlr();
    uint32_t maxDtXferSz = idCmdCtrlr->GetMaxDataXferSize();

    LOG_NRM("Prepare cmds to utilize");
    SharedWritePtr writeCmd = SharedWritePtr(new Write());
    SharedMemBufferPtr writeMem = SharedMemBufferPtr(new MemBuffer());

    SharedReadPtr readCmd = SharedReadPtr(new Read());
    SharedMemBufferPtr readMem = SharedMemBufferPtr(new MemBuffer());

    send_64b_bitmask prpBitmask = (send_64b_bitmask)
        (MASK_PRP1_PAGE | MASK_PRP2_PAGE | MASK_PRP2_LIST);

    DataPattern dataPat[] = {
        DATAPAT_INC_8BIT,
        DATAPAT_CONST_8BIT,
        DATAPAT_INC_16BIT,
        DATAPAT_CONST_16BIT,
        DATAPAT_INC_32BIT,
        DATAPAT_CONST_32BIT
    };
    uint64_t dpArrSize = sizeof(dataPat) / sizeof(dataPat[0]);

    vector<uint32_t> bare = gInformative->GetBareNamespaces();
    for (size_t i = 0; i < bare.size(); i++) {
        namSpcPtr = gInformative->GetIdentifyCmdNamspc(bare[i]);
        uint64_t lbaDataSize = namSpcPtr->GetLBADataSize();
        uint64_t maxWrBlks = (1 << CDW12_NLB_BITS); // 1-based value.
        uint64_t ncap = namSpcPtr->GetValue(IDNAMESPC_NCAP);
        maxWrBlks = (maxWrBlks < ncap) ? maxWrBlks : ncap; // limit by ncap.
        if (maxDtXferSz != 0)
            maxWrBlks = MIN(maxWrBlks, (maxDtXferSz / lbaDataSize));

        writeCmd->SetNSID(bare[i]);
        readCmd->SetNSID(bare[i]);

        // If we execute for every possible LBA, then it will take hrs to
        // complete. So incrementing LBA in powers of 2 is a best effort
        // solution to minimize the execution time.
        // lbaPow2 = {2, 4, 8, 16, 32, 64, ..., 0x10000}
        for (uint64_t lbaPow2 = 2; lbaPow2 <= maxWrBlks; lbaPow2 <<= 1) {
            // nLBA = {(1, 2, 3), (3, 4, 5), ..., (0xFFFF, 0x10000, 0x10001)}
            for (uint64_t nLBA = (lbaPow2 - 1); nLBA <= (lbaPow2 + 1); nLBA++) {
                LOG_NRM("Processing LBA #%ld of %ld", nLBA, maxWrBlks);
                if (nLBA > maxWrBlks)
                    break;
                writeMem->Init(nLBA * lbaDataSize);
                writeCmd->SetPrpBuffer(prpBitmask, writeMem);
                writeCmd->SetNLB(nLBA - 1); // 0 based value.
                writeMem->SetDataPattern(dataPat[(nLBA - 1) % dpArrSize], nLBA);

                readMem->Init(nLBA * lbaDataSize);
                readCmd->SetPrpBuffer(prpBitmask, readMem);
                readCmd->SetNLB(nLBA - 1); // 0 based value.

                enableLog = false;
                if ((nLBA <= 8) || (nLBA >= (maxWrBlks - 8)))
                    enableLog = true;
                work = str(boost::format("NSID.%d.LBA.%ld") % bare[i] % nLBA);

                IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq,
                    iocq, writeCmd, work, enableLog);

                IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq,
                    iocq, readCmd, work, enableLog);

                VerifyDataPat(readCmd, writeMem);
            }
        }
    }

}


void
NLBABare_r10b::VerifyDataPat(SharedReadPtr readCmd,
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

}   // namespace
