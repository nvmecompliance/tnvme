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
#include "maxIOQ_r10b.h"
#include "grpDefs.h"
#include "../Utils/queues.h"
#include "../Utils/io.h"


namespace GrpQueues {


MaxIOQ_r10b::MaxIOQ_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4");
    mTestDesc.SetShort(     "Create and use the max allowed IOCQ/IOSQ's");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Search for 1 of the following namspcs to run test. Find 1st bare "
        "namspc, or find 1st meta namspc, or find 1st E2E namspc. Create 1 "
        "IOSQ to IOCQ pair mapping, and allocate the max number of IOQ's "
        "the DUT supports. All IOCQ's will use polling. Issue a write cmd "
        "sending 1 block and approp supporting meta/E2E if necessary to the "
        "selected namspc at LBA 0, data pattern wordK, and then verify by "
        "reading back for all queues. Use a unique wordK pattern for each "
        "write/read pair for all IOQ pairs.");
}


MaxIOQ_r10b::~MaxIOQ_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


MaxIOQ_r10b::
MaxIOQ_r10b(const MaxIOQ_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


MaxIOQ_r10b &
MaxIOQ_r10b::operator=(const MaxIOQ_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
MaxIOQ_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
MaxIOQ_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     *  \endverbatim
     */
    string work;
    const uint32_t NumEntriesIOQ =     2;

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    // Determine the DUT supports Num IOSQ <= Num IOCQ condition.
    if (gInformative->GetFeaturesNumOfIOSQs() >
        gInformative->GetFeaturesNumOfIOCQs()) {
        // The idea is to issue 1 cmd to all IOSQ's, if a DUT supports >
        // IOSQ's than IOCQ's then this algo needs to be updated since
        // initially the assumption of 1:1 IOSQ:IOCQ was most probable.
        // In order to save time/work ignoring this reported case.
        throw FrmwkEx(HERE, "DUT supports IOSQs > IOCQ's, see source comment here");
    }

    Informative::Namspc namspcData = gInformative->Get1stBareMetaE2E();
    SharedMemBufferPtr writeMem = SharedMemBufferPtr(new MemBuffer());
    SharedMemBufferPtr readMem = SharedMemBufferPtr(new MemBuffer());
    SharedReadPtr readCmd = CreateReadCmd(namspcData, readMem);
    SharedWritePtr writeCmd = SetWriteCmd(namspcData, writeMem);

    vector<SharedIOSQPtr> IOSQVec;
    vector<SharedIOCQPtr> IOCQVec;
    for (uint32_t ioqId = 1; ioqId <= gInformative->GetFeaturesNumOfIOSQs();
        ioqId++) {
        SharedIOCQPtr iocq = Queues::CreateIOCQContigToHdw(mGrpName,
            mTestName, CALC_TIMEOUT_ms(1), asq, acq, ioqId, NumEntriesIOQ,
            false, IOCQ_CONTIG_GROUP_ID, false, 0);
        SharedIOSQPtr iosq = Queues::CreateIOSQContigToHdw(mGrpName,
            mTestName, CALC_TIMEOUT_ms(1), asq, acq, ioqId, NumEntriesIOQ,
            false, IOSQ_CONTIG_GROUP_ID, ioqId, 0);
        IOSQVec.push_back(iosq);
        IOCQVec.push_back(iocq);
    }

    vector <SharedIOCQPtr>::iterator iocq = IOCQVec.begin();
    for (vector <SharedIOSQPtr>::iterator iosq = IOSQVec.begin();
        iosq != IOSQVec.end(); iosq++, iocq++) {
        writeMem->SetDataPattern(DATAPAT_CONST_16BIT, (*iosq)->GetQId());
        work = str(boost::format("dataPattern.0x%04X") % (*iosq)->GetQId());
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), *iosq, *iocq,
            writeCmd, work, true);

        VerifyDataPattern(*iosq, *iocq, readCmd, work);
    }

    iocq = IOCQVec.begin();
    for (vector <SharedIOSQPtr>::iterator iosq = IOSQVec.begin();
        iosq != IOSQVec.end(); iosq++, iocq++) {
        // Delete IOSQ before the IOCQ to comply with spec.
        Queues::DeleteIOSQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
            *iosq, asq, acq);
        Queues::DeleteIOCQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
            *iocq, asq, acq);
    }
}


SharedWritePtr
MaxIOQ_r10b::SetWriteCmd(Informative::Namspc namspcData,
    SharedMemBufferPtr dataPat)
{
    LOG_NRM("Processing write cmd using namspc id %d", namspcData.id);
    uint64_t lbaDataSize = namspcData.idCmdNamspc->GetLBADataSize();
    LBAFormat lbaFormat = namspcData.idCmdNamspc->GetLBAFormat();

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


SharedReadPtr
MaxIOQ_r10b::CreateReadCmd(Informative::Namspc namspcData,
    SharedMemBufferPtr dataPat)
{
    LOG_NRM("Processing read cmd using namspc id %d", namspcData.id);
    uint64_t lbaDataSize = namspcData.idCmdNamspc->GetLBADataSize();
    LBAFormat lbaFormat = namspcData.idCmdNamspc->GetLBAFormat();

    SharedReadPtr readCmd = SharedReadPtr(new Read());
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
        readCmd->AllocMetaBuffer();
        break;
    case Informative::NS_METAI:
        dataPat->Init(lbaDataSize + lbaFormat.MS);
        break;
    case Informative::NS_E2ES:
    case Informative::NS_E2EI:
        throw FrmwkEx(HERE, "Deferring work to handle this case in future");
        break;
    }

    readCmd->SetPrpBuffer(prpBitmask, dataPat);
    readCmd->SetNSID(namspcData.id);
    readCmd->SetNLB(0);
    return readCmd;
}


void
MaxIOQ_r10b::VerifyDataPattern(SharedIOSQPtr iosq, SharedIOCQPtr iocq,
    SharedReadPtr readCmd, string qualifier)
{
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq,
        readCmd, qualifier, true);

    LOG_NRM("Compare read vs written data to verify");
    const uint16_t *rdBuffPtr = (const uint16_t *)readCmd->GetROPrpBuffer();
    for (uint64_t i = 0; i < (readCmd->GetPrpBufferSize() / sizeof(uint16_t));
        i++) {
        if (*rdBuffPtr++ != (uint16_t)iosq->GetQId()) {
            readCmd->Dump(
                FileSystem::PrepDumpFile(mGrpName, mTestName, "ReadPayload"),
                "Data read from media miscompared from written");
            throw FrmwkEx(HERE, "Read data mismatch for SQID #%d, pattern 0x%02X",
                iosq->GetQId(), (uint16_t)iosq->GetQId());
        }
    }
}

}   // namespace
