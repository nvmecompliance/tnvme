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

#include "cidAcceptedIOSQ_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/io.h"
#include "../Utils/irq.h"


namespace GrpGeneralCmds {

#define MAX_CMDS        (65536 + 1)
#define NUM_IO_SQS      2


CIDAcceptedIOSQ_r10b::CIDAcceptedIOSQ_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4");
    mTestDesc.SetShort(     "Verify all CID values accepted in IOSQs.");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Search for 1 of the following namspcs to run test. Find 1st bare "
        "namspc, or find 1st meta namspc, or find 1st E2E namspc. Issue "
        "identical write cmd starting at LBA 0, sending a single block with "
        "approp meta/E2E requirements if necessary. Create 2 IOSQ's assoc to "
        "single IOCQ. Issue dual write cmds (65536 + 1) times into each IOSQ, "
        "alternating IOSQ's, and verify that the dnvme assigned CID values "
        "are unique each time for each IOSQ. Each command must completed "
        "in success and be reaped from the IOCQ.");
}


CIDAcceptedIOSQ_r10b::~CIDAcceptedIOSQ_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


CIDAcceptedIOSQ_r10b::
CIDAcceptedIOSQ_r10b(const CIDAcceptedIOSQ_r10b &other) :
    Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


CIDAcceptedIOSQ_r10b &
CIDAcceptedIOSQ_r10b::operator=(const CIDAcceptedIOSQ_r10b
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
CIDAcceptedIOSQ_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
CIDAcceptedIOSQ_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * None.
     * \endverbatim
     */
    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Create ACQ and ASQ objects which have test life time");
    SharedACQPtr acq = CAST_TO_ACQ(SharedACQPtr(new ACQ(gDutFd)))
    acq->Init(5);
    SharedASQPtr asq = CAST_TO_ASQ(SharedASQPtr(new ASQ(gDutFd)))
    asq->Init(5);

    IRQ::SetAnySchemeSpecifyNum(2);     // throws upon error

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);

    vector<SharedIOSQPtr> iosqs;
    SharedIOCQPtr iocq;
    InitTstRsrcs(asq, acq, iosqs, iocq);
    SharedWritePtr writeCmd = CreateWriteCmd();

    LOG_NRM("Learn initial unique command id assigned by dnvme");
    uint16_t curCID;
    vector <SharedIOSQPtr>::iterator iosq;
    for (iosq = iosqs.begin(); iosq != iosqs.end(); iosq++)
        (*iosq)->Send(writeCmd, curCID);

    uint16_t prevCID = curCID;
    for (uint32_t nCmds = 0; nCmds < MAX_CMDS; nCmds++) {
        for (iosq = iosqs.begin(); iosq != iosqs.end(); iosq++) {
            LOG_NRM("(nCmds, curCID, prevCID, SQ ID) = (%d, %d, %d, %d)",
                nCmds, curCID, prevCID, (*iosq)->GetQId());
            (*iosq)->Ring();
            ReapVerifyCID(*iosq, iocq, prevCID);

            (*iosq)->Send(writeCmd, curCID);
            if (curCID != (uint16_t)(prevCID + 1)) {
                (*iosq)->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName,
                    "iosq.fail." + (*iosq)->GetQId()), "Dump Entire IOSQ");
                iocq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName,
                    "iocq.fail." + iocq->GetQId()), "Dump Entire IOCQ");
                throw FrmwkEx(HERE, "curCID(%d) != (prevCID + 1)(%d)", curCID,
                    (prevCID + 1));
            }
        }
        prevCID = curCID;
    }
}


void
CIDAcceptedIOSQ_r10b::InitTstRsrcs(SharedASQPtr asq, SharedACQPtr acq,
    vector <SharedIOSQPtr> &iosqs, SharedIOCQPtr &iocq)
{
    uint8_t iocqes = (gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_CQES) & 0xf);
    uint8_t iosqes = (gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_SQES) & 0xf);

    gCtrlrConfig->SetIOCQES(iocqes);
    gCtrlrConfig->SetIOSQES(iosqes);

    uint32_t numIOSQs = (NUM_IO_SQS <= gInformative->GetFeaturesNumOfIOSQs()) ?
        NUM_IO_SQS : gInformative->GetFeaturesNumOfIOSQs();

    LOG_NRM("Initialize test resources.");
    const uint32_t nEntriesIOQ = 2; // minimum Q entries always supported.
    SharedIOSQPtr iosq;
    if (Queues::SupportDiscontigIOQ() == true) {
        SharedMemBufferPtr iocqMem =  SharedMemBufferPtr(new MemBuffer());
        iocqMem->InitOffset1stPage((nEntriesIOQ * (1 << iocqes)), 0, true);

        iocq = Queues::CreateIOCQDiscontigToHdw(mGrpName, mTestName,
            CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, nEntriesIOQ, false,
            IOCQ_GROUP_ID, true, 1, iocqMem);

        for (uint32_t iosqId = 1; iosqId <= numIOSQs; iosqId++) {
            SharedMemBufferPtr iosqMem = SharedMemBufferPtr(new MemBuffer());
            iosqMem->InitOffset1stPage((nEntriesIOQ * (1 << iosqes)), 0, true);
            iosq = Queues::CreateIOSQDiscontigToHdw(mGrpName, mTestName,
                CALC_TIMEOUT_ms(1), asq, acq, iosqId, nEntriesIOQ, false,
                IOSQ_GROUP_ID, IOQ_ID, 0, iosqMem);
            iosqs.push_back(iosq);
        }
    } else {
       iocq = Queues::CreateIOCQContigToHdw(mGrpName, mTestName,
           CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, nEntriesIOQ, false,
           IOCQ_GROUP_ID, true, 1);

       for (uint32_t iosqId = 1; iosqId <= numIOSQs; iosqId++) {
           iosq = Queues::CreateIOSQContigToHdw(mGrpName, mTestName,
               CALC_TIMEOUT_ms(1), asq, acq, iosqId, nEntriesIOQ, false,
               IOSQ_GROUP_ID, IOQ_ID, 0);
           iosqs.push_back(iosq);
       }
    }
}


SharedWritePtr
CIDAcceptedIOSQ_r10b::CreateWriteCmd()
{
    Informative::Namspc namspcData = gInformative->Get1stBareMetaE2E();
    LBAFormat lbaFormat = namspcData.idCmdNamspc->GetLBAFormat();

    SharedMemBufferPtr wrMemBuf = SharedMemBufferPtr(new MemBuffer());
    uint64_t lbaDataSize = namspcData.idCmdNamspc->GetLBADataSize();

    SharedWritePtr writeCmd = SharedWritePtr(new Write());
    send_64b_bitmask prpBitmask = (send_64b_bitmask)(MASK_PRP1_PAGE
        | MASK_PRP2_PAGE | MASK_PRP2_LIST);

    switch (namspcData.type) {
    case Informative::NS_BARE:
        wrMemBuf->Init(lbaDataSize);
        break;
    case Informative::NS_METAS:
        wrMemBuf->Init(lbaDataSize);
        if (gRsrcMngr->SetMetaAllocSize(lbaFormat.MS) == false)
            throw FrmwkEx(HERE);
        writeCmd->AllocMetaBuffer();
        break;
    case Informative::NS_METAI:
        wrMemBuf->Init(lbaDataSize + lbaFormat.MS);
        break;
    case Informative::NS_E2ES:
    case Informative::NS_E2EI:
        throw FrmwkEx(HERE, "Deferring work to handle this case in future");
        break;
    }

    writeCmd->SetPrpBuffer(prpBitmask, wrMemBuf);
    writeCmd->SetNSID(namspcData.id);

    return writeCmd;
}


void
CIDAcceptedIOSQ_r10b::ReapVerifyCID(SharedIOSQPtr iosq, SharedIOCQPtr iocq,
    uint16_t expCID)
{
    uint32_t isrCount;
    uint32_t ceRemain;
    uint32_t numReaped;
    uint32_t numCE;

    if (iocq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(1), 1, numCE,
        isrCount) == false) {
        iocq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "iocq.fail"),
            "Dump Entire IOCQ");
        iosq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "iosq.fail"),
            "Dump Entire IOSQ");
        throw FrmwkEx(HERE, "Unable to see CEs for issued cmd");
    }

    SharedMemBufferPtr ceMem = SharedMemBufferPtr(new MemBuffer());
    if ((numReaped = iocq->Reap(ceRemain, ceMem, isrCount, numCE, true)) != 1) {
        iocq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "iocq.fail"),
            "Dump Entire IOCQ");
        iosq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "iosq.fail"),
            "Dump Entire IOSQ");
        throw FrmwkEx(HERE, "Unable to reap on IOCQ");
    }

    union CE *ce = (union CE *)ceMem->GetBuffer();
    ProcessCE::Validate(*ce);  // throws upon error

    if (ce->n.CID != expCID) {
        iocq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "iocq.fail"),
            "Dump Entire IOCQ");
        iosq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "iosq.fail"),
            "Dump Entire IOSQ");
        throw FrmwkEx(HERE, "Received CID %d but expected %d", ce->n.CID,
            expCID);
    }

    if (ce->n.SQID != iosq->GetQId()) {
        iocq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "iocq.fail"),
            "Dump Entire IOCQ");
        iosq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "iosq.fail"),
            "Dump Entire IOSQ");
        throw FrmwkEx(HERE, "Rx'd SDID %d but expt'd %d", ce->n.SQID,
            iosq->GetQId());
    }

}

}   // namespace
