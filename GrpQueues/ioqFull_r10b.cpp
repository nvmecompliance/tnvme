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
#include "ioqFull_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/io.h"

namespace GrpQueues {


IOQFull_r10b::IOQFull_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4");
    mTestDesc.SetShort(     "Issue cmds until both IOSQ and IOCQ fill up.");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Search for 1 of the following namspcs to run test. Find 1st bare "
        "namspc, or find 1st meta namspc, or find 1st E2E namspc. Create 2 "
        "cases where size: {IOSQ=IOCQ, IOSQ=IOCQ+1}, however within each "
        "case, 3 samples of the queue pairs are {min, mid, max} sized. Issue "
        "NVM write cmd, sending 1 block and approp supporting meta/E2E if "
        "necessary to the selected namspc at LBA 0. Submit ((IOSQ.size-1) "
        "cmds into IOSQ always fills both queues, submit, ring each cmd, wait "
        "for CE to arrive, continue pattern. Verify in = case all CE's "
        "arrive, in > case all but 1 CE's arrive in IOCQ.");
}


IOQFull_r10b::~IOQFull_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


IOQFull_r10b::
IOQFull_r10b(const IOQFull_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


IOQFull_r10b &
IOQFull_r10b::operator=(const IOQFull_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
IOQFull_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
IOQFull_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     *  \endverbatim
     */

    uint64_t ctrlCapReg;
    LOG_NRM("Determine the max IOQ entries supported");
    if (gRegisters->Read(CTLSPC_CAP, ctrlCapReg) == false)
        throw FrmwkEx(HERE, "Unable to determine MQES");
    uint32_t maxIOQEntries = (ctrlCapReg & CAP_MQES);
    maxIOQEntries += 1;      // convert to 1-based

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    SharedWritePtr writeCmd = SetWriteCmd();

    LOG_NRM("Case 1 - IOSQ = IOCQ (min, middle and max)");
    IOQFull(2, 2, asq, acq, writeCmd);
    IOQFull((maxIOQEntries/2), (maxIOQEntries/2), asq, acq, writeCmd);
    IOQFull(maxIOQEntries, maxIOQEntries, asq, acq, writeCmd);

    LOG_NRM("Case 2 - IOSQ =  IOCQ + 1 (min , middle and max)");
    IOQFull(3, 2, asq, acq, writeCmd);
    IOQFull((maxIOQEntries/2), ((maxIOQEntries/2) - 1), asq, acq, writeCmd);
    IOQFull(maxIOQEntries, (maxIOQEntries - 1), asq, acq, writeCmd);

}


void
IOQFull_r10b::IOQFull(uint32_t numIOSQEntries, uint32_t numIOCQEntries,
        SharedASQPtr asq, SharedACQPtr acq, SharedWritePtr writeCmd)
{
    string work;
    uint32_t numCE;
    uint32_t isrCount;
    SharedIOCQPtr iocq;
    SharedIOSQPtr iosq;
    uint16_t uniqueId;

    if (Queues::SupportDiscontigIOQ() == true) {
        uint8_t iocqes = (gInformative->GetIdentifyCmdCtrlr()->
            GetValue(IDCTRLRCAP_CQES) & 0xf);
        SharedMemBufferPtr iocqBackedMem = SharedMemBufferPtr(new MemBuffer());
        iocqBackedMem->
            InitOffset1stPage((numIOCQEntries * (1 << iocqes)), 0, true);
        iocq = Queues::CreateIOCQDiscontigToHdw(mGrpName,
            mTestName, CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, numIOCQEntries,
            false, IOCQ_CONTIG_GROUP_ID, false, 1, iocqBackedMem);

        uint8_t iosqes = (gInformative->GetIdentifyCmdCtrlr()->
            GetValue(IDCTRLRCAP_SQES) & 0xf);
        SharedMemBufferPtr iosqBackedMem = SharedMemBufferPtr(new MemBuffer());
        iosqBackedMem->
            InitOffset1stPage((numIOSQEntries * (1 << iosqes)), 0, true);
        iosq = Queues::CreateIOSQDiscontigToHdw(mGrpName,
            mTestName, CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, numIOSQEntries,
            false, IOSQ_CONTIG_GROUP_ID, IOQ_ID, 0, iosqBackedMem);
    } else {
        iocq = Queues::CreateIOCQContigToHdw(mGrpName,
            mTestName, CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, numIOCQEntries,
            false, IOCQ_CONTIG_GROUP_ID, false, 1);
        iosq = Queues::CreateIOSQContigToHdw(mGrpName,
            mTestName, CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, numIOSQEntries,
            false, IOSQ_CONTIG_GROUP_ID, IOQ_ID, 0);
    }

    uint32_t nCmdsToSubmit = (numIOSQEntries - 1);
    LOG_NRM("Send #%d cmds to hdw via IOSQ", nCmdsToSubmit);
    for (uint32_t nCmds = 0; nCmds < nCmdsToSubmit; nCmds++) {
        LOG_NRM("Sending #%d of #%d NVM Write Cmds thru IOSQ", (nCmds + 1),
            nCmdsToSubmit);
        iosq->Send(writeCmd, uniqueId);
        iosq->Ring();

        LOG_NRM("Wait for the CE to arrive in IOCQ");
        if (iocq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(1), (nCmds + 1),
            numCE, isrCount) == false) {

            // if (iosq size == (iocq size + 1)); last CE will not arrive.
            if ((numIOSQEntries == (numIOCQEntries + 1)) &&
                (nCmds == (nCmdsToSubmit - 1))) {

                LOG_NRM("Reap one element from IOCQ to make room for last CE.");
                IO::ReapCE(iocq, 1, isrCount, mGrpName, mTestName, "IOCQCE",
                    CESTAT_SUCCESS);

                if (iocq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(1), nCmds,
                    numCE, isrCount)) {
                    break;  // Seen the last CE, all if fine
                }
            }
            work = str(boost::format("Dump entire CQ %d") % iocq->GetQId());
            iocq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName,
                "iocq." + writeCmd->GetName()), work);
            throw FrmwkEx(HERE, "Unable to see CE for issued cmd #%d", nCmds + 1);

        } else if (numCE != nCmds + 1) {
            work = str(boost::format("Dump entire CQ %d") % iocq->GetQId());
            iocq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName,
                "iocq." + writeCmd->GetName()), work);
            throw FrmwkEx(HERE, "Missing last CE, #%d cmds of #%d received",
                nCmds + 1, numCE);
        }
    }

    LOG_NRM(" Delete IOSQ before the IOCQ to comply with spec.");
    Queues::DeleteIOSQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
        iosq, asq, acq);
    Queues::DeleteIOCQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
        iocq, asq, acq);
}


SharedWritePtr
IOQFull_r10b::SetWriteCmd()
{
    Informative::Namspc namspcData = gInformative->Get1stBareMetaE2E();
    LOG_NRM("Processing write cmd using namspc id %d", namspcData.id);
    LBAFormat lbaFormat = namspcData.idCmdNamspc->GetLBAFormat();

    LOG_NRM("Create data pattern to write to media");
    SharedMemBufferPtr dataPat = SharedMemBufferPtr(new MemBuffer());
    uint64_t lbaDataSize = namspcData.idCmdNamspc->GetLBADataSize();

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

}   // namespace
