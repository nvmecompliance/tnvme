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

#include "partialReapMSIX_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "createResources_r10b.h"
#include "../Utils/queues.h"
#include "../Utils/io.h"

#define IOQ_ID                      1
#define NUM_CMDS_ISSUE              10
#define NUM_IOQ_ENTRY               (NUM_CMDS_ISSUE + 1)


namespace GrpInterrupts {


PartialReapMSIX_r10b::PartialReapMSIX_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4,7");
    mTestDesc.SetShort(     "Verify correct behavior during partial IOCQ reaping for MSI-X");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Only allowed to execute test if the DUT supports MSI-X IRQ's by "
        "reporting the MSIXCAP PCI structures.  Search for 1 of the following "
        "namspcs to run test. Find 1st bare namspc, or find 1st meta namspc, "
        "or find 1st E2E namspc. Issue identical write cmds starting at LBA 0. "
        "Create a single IOSQ/IOCQ pair using IRQ 0. Issue 10 write cmds w/o "
        "reaping the CE's, 1 block each, data pattern is a don’t care. Wait "
        "until all 10 CE's arrive in IOCQ, reap 1 element and verify the "
        "number of IRQ's fired is still 3, 2 IRQ's just for creating the IOQ "
        "pairs, the extra IRQ for the 1st reaped write, and a 4th and final "
        "IRQ for the PBA pending bit being set for cmds 2 to 10.");
}


PartialReapMSIX_r10b::~PartialReapMSIX_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


PartialReapMSIX_r10b::
PartialReapMSIX_r10b(const PartialReapMSIX_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


PartialReapMSIX_r10b &
PartialReapMSIX_r10b::operator=(const PartialReapMSIX_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
PartialReapMSIX_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
PartialReapMSIX_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Requires test createResources_r10b to execute 1st
     * \endverbatim
     */
    bool capable;
    char work[25];
    uint64_t reg;
    uint16_t numIrqSupport;
    uint16_t numIrqs = 1;
    uint16_t uniqueId;
    uint32_t isrCount;
    uint32_t numCE;


    // Only allowed to execute if DUT supports MSI-X IRQ's
    if (gCtrlrConfig->IsMSIXCapable(capable, numIrqSupport) == false)
        throw FrmwkEx(HERE);
    else if (capable == false) {
        LOG_WARN("DUT does not support MSI-X IRQ's; unable to execute test");
        return;
    }

    // This requirement must be met, if not then DUT is most likely at fault
    if (gRegisters->Read(CTLSPC_CAP, reg) == false)
        throw FrmwkEx(HERE, "Unable to determine CAP.MQES");
    reg &= CAP_MQES;
    reg += 1;      // convert to 1-based
    if (reg < (uint64_t)NUM_IOQ_ENTRY) {
        LOG_WARN("Desired to support >= %d elements in IOQ's for test",
            NUM_IOQ_ENTRY);
        return;
    }

    LOG_NRM("Setup the necessary IRQ's");
    if (gCtrlrConfig->SetState(ST_DISABLE) == false)
        throw FrmwkEx(HERE);
    if (gCtrlrConfig->SetIrqScheme(INT_MSIX, numIrqs) == false) {
        throw FrmwkEx(HERE,
            "Unable to use %d IRQ's, but DUT reports it supports", numIrqs);
    }
    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    LOG_NRM("Create the IOQ pair which is needed");
    gCtrlrConfig->SetIOCQES(gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_CQES) & 0xf);
    SharedIOCQPtr iocq = Queues::CreateIOCQContigToHdw(mGrpName, mTestName,
        CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, NUM_IOQ_ENTRY, false, "",
        true, 0);

    gCtrlrConfig->SetIOSQES(gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_SQES) & 0xf);
    SharedIOSQPtr iosq = Queues::CreateIOSQContigToHdw(mGrpName, mTestName,
        CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, NUM_IOQ_ENTRY, false, "",
        IOQ_ID, 0);

    // NOTE: We are overloading IRQ 0, it is being used by ACQ and we have
    //       created 1 IOQ's, thus to start out the ACQ will already have 2
    //       IRQs. The following algo must add this usage.
    uint32_t anticipatedIrqs = (2 + 1);

    SharedWritePtr writeCmd = CreateCmd();
    for (unsigned i = 1; i <= NUM_CMDS_ISSUE; i++) {
        LOG_NRM("Sending write cmd %d", i);
        iosq->Send(writeCmd, uniqueId);
        iosq->Ring();

        LOG_NRM("Wait for cmd to be processed");
        if (iocq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(1), i, numCE,
            isrCount) == false) {

            iocq->Dump(
                FileSystem::PrepDumpFile(mGrpName, mTestName, "acq",
                "error"), "Unable to determine if CE's arrived");
            throw FrmwkEx(HERE, "Only detected %d CE's arriving", numCE);
        } else if (numCE != i) {
            iocq->Dump(
                FileSystem::PrepDumpFile(mGrpName, mTestName, "acq",
                "notEnough"), "Test requires seeing all CE's");
            throw FrmwkEx(HERE,
                "The anticipated %d CE's have not arrived", i);
        } else if (isrCount != anticipatedIrqs) {
            // 1 IRQ per cmd did not occur
            iocq->Dump(
                FileSystem::PrepDumpFile(mGrpName, mTestName, "acq",
                "irqBad"), "Test requires seeing all correct num of IRQ's");
            throw FrmwkEx(HERE,
                "The anticipated %d IRQ', but detected %d", anticipatedIrqs,
                isrCount);
        }
    }

    // There is an active IRQ outstanding and another to arrive due to the
    // PBA pending bit being set. The pending bit makes the 4th IRQ arrive
    // after reaping a single cmd.
    LOG_NRM("Start reaping and validate %d IRQ's", anticipatedIrqs);
    for (int i = 0; i < NUM_CMDS_ISSUE; i++) {
        snprintf(work, sizeof(work), "cmd.%d", i);
        IO::ReapCE(iocq, 1, isrCount, mGrpName, mTestName, work);
        if (isrCount != anticipatedIrqs) {
            iocq->Dump(
                FileSystem::PrepDumpFile(mGrpName, mTestName, "acq",
                work), "Number of IRQ's changed while reaping");
            throw FrmwkEx(HERE,
                "Anticipated %d IRQ's; but only see %d",
                    anticipatedIrqs, isrCount);
        }

        // Now account for new IRQ's fired due to the pending bit being handled
        // by the DUT, but delay processing so that latency in handling this
        // extra IRQ is not the cause of a test failure. Rather make the
        // absence of the IRQ be the failure, thus delaying is OK.
        anticipatedIrqs++;
        sleep(1);
    }
}


SharedWritePtr
PartialReapMSIX_r10b::CreateCmd()
{
    Informative::Namspc namspcData = gInformative->Get1stBareMetaE2E();
    LBAFormat lbaFormat = namspcData.idCmdNamspc->GetLBAFormat();
    LOG_NRM("Processing write cmd using namspc id %d", namspcData.id);

    ConstSharedIdentifyPtr namSpcPtr = namspcData.idCmdNamspc;
    uint64_t lbaDataSize = namSpcPtr->GetLBADataSize();;
    SharedMemBufferPtr dataPat = SharedMemBufferPtr(new MemBuffer());

    SharedWritePtr writeCmd = SharedWritePtr(new Write());
    send_64b_bitmask prpBitmask = (send_64b_bitmask)(MASK_PRP1_PAGE |
        MASK_PRP2_PAGE | MASK_PRP2_LIST);

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
    return writeCmd;
}


}   // namespace
