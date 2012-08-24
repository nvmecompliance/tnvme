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

#include <string.h>
#include <boost/format.hpp>
#include "datasetMgmt_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/io.h"


namespace GrpNVMWriteReadCombo {

#define CDW13_DSM_BITS          8

DatasetMgmt_r10b::DatasetMgmt_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Verify dataset mgmt operations.");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Search for 1 of the following namspcs to run test. Find 1st bare "
        "namspc, or find 1st meta namspc, or find 1st E2E namspc. Issue "
        "identical write cmd at LBA 0, sending 1 block with approp meta/E2E "
        "requirements if necessary, where write executes the ALGO listed "
        "below; subsequently after each write a read must verify the data "
        "pattern for success, where by the read cmd must be created with the "
        "identical parameters that the write occurred. Each write cmd must "
        "alternate the data pattern between dword++ and dwordK. "
        "ALGO) Vary the cmd's DW13.DSM field for all possible permutations"
        "of values except any reserved values. Testing to verify these "
        "attributes don't affect the integrity of data written and then "
        "read to/from media.");
}


DatasetMgmt_r10b::~DatasetMgmt_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


DatasetMgmt_r10b::
DatasetMgmt_r10b(const DatasetMgmt_r10b &other) :
    Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


DatasetMgmt_r10b &
DatasetMgmt_r10b::operator=(const DatasetMgmt_r10b
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
DatasetMgmt_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
DatasetMgmt_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */
    string work;

    // Lookup objs which were created in a prior test within group
    SharedIOSQPtr iosq = CAST_TO_IOSQ(gRsrcMngr->GetObj(IOSQ_GROUP_ID));
    SharedIOCQPtr iocq = CAST_TO_IOCQ(gRsrcMngr->GetObj(IOCQ_GROUP_ID));

    Informative::Namspc namspcData = gInformative->Get1stBareMetaE2E();
    LBAFormat lbaFormat = namspcData.idCmdNamspc->GetLBAFormat();
    uint64_t lbaDataSize = namspcData.idCmdNamspc->GetLBADataSize();

    SharedWritePtr writeCmd = SharedWritePtr(new Write());
    SharedMemBufferPtr writeMem = SharedMemBufferPtr(new MemBuffer());

    SharedReadPtr readCmd = SharedReadPtr(new Read());
    SharedMemBufferPtr readMem = SharedMemBufferPtr(new MemBuffer());

    send_64b_bitmask prpBitmask = (send_64b_bitmask)(MASK_PRP1_PAGE
        | MASK_PRP2_PAGE | MASK_PRP2_LIST);

    switch (namspcData.type) {
    case Informative::NS_BARE:
        writeMem->Init(lbaDataSize);
        readMem->Init(lbaDataSize);
        break;
    case Informative::NS_METAS:
        writeMem->Init(lbaDataSize);
        readMem->Init(lbaDataSize);
        if (gRsrcMngr->SetMetaAllocSize(lbaFormat.MS) == false)
            throw FrmwkEx(HERE);
        writeCmd->AllocMetaBuffer();
        readCmd->AllocMetaBuffer();
        break;
    case Informative::NS_METAI:
        writeMem->Init(lbaDataSize + lbaFormat.MS);
        readMem->Init(lbaDataSize + lbaFormat.MS);
        break;
    case Informative::NS_E2ES:
    case Informative::NS_E2EI:
        throw FrmwkEx(HERE, "Deferring work to handle this case in future");
        break;
    }

    writeCmd->SetPrpBuffer(prpBitmask, writeMem);
    writeCmd->SetNSID(namspcData.id);
    writeCmd->SetNLB(0);

    readCmd->SetPrpBuffer(prpBitmask, readMem);
    readCmd->SetNSID(namspcData.id);
    readCmd->SetNLB(0);

    DataPattern dataPat[] = {
        DATAPAT_INC_32BIT,
        DATAPAT_CONST_32BIT
    };
    uint64_t dpArrSize = sizeof(dataPat) / sizeof(dataPat[0]);

    for (uint64_t dsmAtr = 0; dsmAtr < (1 << CDW13_DSM_BITS); dsmAtr++) {
        // skip reserved bits in dataset management attributes.
        if ((dsmAtr & 0xF) >= 0x09)
            continue;

        switch (namspcData.type) {
        case Informative::NS_BARE:
            writeMem->SetDataPattern(dataPat[dsmAtr % dpArrSize], dsmAtr);
            break;
        case Informative::NS_METAS:
            writeMem->SetDataPattern(dataPat[dsmAtr % dpArrSize], dsmAtr);
            writeCmd->SetMetaDataPattern(dataPat[dsmAtr % dpArrSize], dsmAtr);
            break;
        case Informative::NS_METAI:
            writeMem->SetDataPattern(dataPat[dsmAtr % dpArrSize], dsmAtr);
            break;
        case Informative::NS_E2ES:
        case Informative::NS_E2EI:
            throw FrmwkEx(HERE, "Deferring work to handle this case in future");
            break;
        }

        // Set CDW13.DSM field to different values.
        writeCmd->SetByte((uint8_t)dsmAtr, 13, 0);
        readCmd->SetByte((uint8_t)dsmAtr, 13, 0);

        work = str(boost::format("dsm.%Xh") % dsmAtr);
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq,
            iocq, writeCmd, work, true);

        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq,
            iocq, readCmd, work, true);

        VerifyDataPat(readCmd, writeCmd);
    }
}


void
DatasetMgmt_r10b::VerifyDataPat(SharedReadPtr readCmd, SharedWritePtr writeCmd)
{
    LOG_NRM("Compare read vs written data to verify");
    SharedMemBufferPtr rdPayload = readCmd->GetRWPrpBuffer();
    SharedMemBufferPtr wrPayload = writeCmd->GetRWPrpBuffer();
    if (rdPayload->Compare(wrPayload) == false) {
        readCmd->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "ReadCmd"),
            "Read command");
        writeCmd->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "WriteCmd"),
            "Write command");
        throw FrmwkEx(HERE, "Data miscompare");
    }

    // If meta data is allocated then compare meta data.
    if (writeCmd->GetMetaBuffer() != NULL) {
        const uint8_t *metaRdBuff = readCmd->GetMetaBuffer();
        const uint8_t *metaWrBuff = writeCmd->GetMetaBuffer();
        if (memcmp(metaRdBuff, metaWrBuff, writeCmd->GetMetaBufferSize())) {
            readCmd->Dump(
                FileSystem::PrepDumpFile(mGrpName, mTestName, "ReadCmdMeta"),
                "Read command with meta data");
            writeCmd->Dump(
                FileSystem::PrepDumpFile(mGrpName, mTestName, "WriteCmdMeta"),
                "Write command with meta data");
            throw FrmwkEx(HERE, "Meta data miscompare");
        }
    }
}

}   // namespace
