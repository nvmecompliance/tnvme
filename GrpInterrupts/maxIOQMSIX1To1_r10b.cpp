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
#include "maxIOQMSIX1To1_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/irq.h"
#include "../Utils/io.h"


namespace GrpInterrupts {


MaxIOQMSIX1To1_r10b::MaxIOQMSIX1To1_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4,7");
    mTestDesc.SetShort(     "Create and use the max allowed IOCQ/IOSQ's under MSI-X (1:1)");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Only allowed to execute test if the DUT supports MSI-X IRQ's by "
        "reporting the MSIXCAP PCI structures. Search for 1 of the following "
        "namspcs to run test. Find 1st bare namspc, or find 1st meta namspc, "
        "or find 1st E2E namspc. Issue identical write cmd starting at LBA 0. "
        "Determine X, where X is the lesser of MSIXCAP.MXC.TS + 1, or the max "
        "number of IOQ's the DUT supports. Create 1 IOSQ:IOCQ pair mappings, "
        "and allocate X number of queue pairs. Alternate IOCQ's indicating "
        "usage of polling verses IRQ's. Issue a write of 1 block, "
        "data pattern byteK, and then verify by reading back for all queues. "
        "For each IOCQ using IRQ's verify that each one has exactly 2 IRQ's "
        "fired. Redo identical test but this time delete only the IOQ's using "
        "polling scheme and recreate them again using a unique IRQ, i.e. the "
        "vectors which were skipping in 1st run of test should be used so that "
        "all queues are contiguously consuming all IRQ vectors.");
}


MaxIOQMSIX1To1_r10b::~MaxIOQMSIX1To1_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


MaxIOQMSIX1To1_r10b::
MaxIOQMSIX1To1_r10b(const MaxIOQMSIX1To1_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


MaxIOQMSIX1To1_r10b &
MaxIOQMSIX1To1_r10b::operator=(const MaxIOQMSIX1To1_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
MaxIOQMSIX1To1_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
MaxIOQMSIX1To1_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */
    bool capable;
    uint16_t numIrqSupport;
    uint32_t isrCount;

    LOG_NRM("Only allowed to execute if DUT supports MSI-X IRQ's");
    if (gCtrlrConfig->IsMSIXCapable(capable, numIrqSupport) == false)
        throw FrmwkEx(HERE);
    else if (capable == false) {
        LOG_NRM("DUT does not support MSI-X IRQ's; unable to execute test");
        return;
    }

    if (gCtrlrConfig->SetState(ST_DISABLE) == false)
        throw FrmwkEx(HERE);

    uint16_t X =
        MIN(numIrqSupport, MIN((gInformative->GetFeaturesNumOfIOSQs() + 1),
        (gInformative->GetFeaturesNumOfIOCQs() + 1)));

    LOG_NRM("Setting MSI-X with #%d irq vectors", X);
    if (gCtrlrConfig->SetIrqScheme(INT_MSIX, X) == false) {
        LOG_NRM("Unable to set MSI-X scheme with num irqs #%d", X);
        throw FrmwkEx(HERE);
    }

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Lookup objs which were created in a prior test within group");
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

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
        readMem->Init(lbaDataSize  + lbaFormat.MS);
        break;
    case Informative::NS_E2ES:
    case Informative::NS_E2EI:
        throw FrmwkEx(HERE, "Deferring work to handle this case in future");
        break;
    }
    writeCmd->SetPrpBuffer(prpBitmask, writeMem);
    writeCmd->SetNSID(namspcData.id);

    readCmd->SetPrpBuffer(prpBitmask, readMem);
    readCmd->SetNSID(namspcData.id);

    gCtrlrConfig->SetIOCQES((gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_CQES) & 0xf));
    gCtrlrConfig->SetIOSQES((gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_SQES) & 0xf));

    SharedIOCQPtr iocq;
    SharedIOSQPtr iosq;
    vector<SharedIOSQPtr> iosqs;
    vector<SharedIOCQPtr> iocqs;
    bool enableIrq;

    LOG_NRM("Create ioqs with polling and isr set alternatively.");
    for (uint16_t ioqId = 1; ioqId < X; ioqId++) {
        enableIrq = (ioqId % 2 == 0) ? true : false;
        CreateIOQs(asq, acq, ioqId, enableIrq, iosq, iocq);
        iosqs.push_back(iosq);
        iocqs.push_back(iocq);
    }

    LOG_NRM("Send two commands and verify isr count.");
    for(uint16_t i = 0; i < iosqs.size(); i++) {
        writeMem->SetDataPattern(DATAPAT_CONST_8BIT, (iosqs[i])->GetQId());
        writeCmd->SetMetaDataPattern(DATAPAT_CONST_8BIT, (iosqs[i])->GetQId());

        SendCmd(iosqs[i], iocqs[i], writeCmd, isrCount);
        SendCmd(iosqs[i], iocqs[i], readCmd, isrCount);

        if ((iocqs[i])->GetIrqEnabled() == true) {
            if (isrCount != 2)
                throw FrmwkEx(HERE, "Invalid isrCount %d expected 2", isrCount);
        } else if (isrCount != 0) {
            throw FrmwkEx(HERE, "Invalid isrCount %d expected 0", isrCount);
        }
        VerifyData(readCmd, writeCmd);
    }

    LOG_NRM("Replace polling IOQs with interrupt IOQs.");
    for(uint16_t i = 0; i < iosqs.size(); i++) {
        if ((iocqs[i])->GetIrqEnabled() == false) {
            uint16_t ioqId = (iosqs[i])->GetQId();
            Queues::DeleteIOSQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
                iosqs[i], asq, acq);
            Queues::DeleteIOCQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
                iocqs[i], asq, acq);

            CreateIOQs(asq, acq, ioqId, true, iosq, iocq);
            iosqs[i] = iosq;
            iocqs[i] = iocq;
        }
    }

    LOG_NRM("Resends two commands and verify isr count.");
    for(uint16_t i = 0; i < iosqs.size(); i++) {
        writeMem->SetDataPattern(DATAPAT_CONST_8BIT, (iosqs[i])->GetQId());
        writeCmd->SetMetaDataPattern(DATAPAT_CONST_8BIT, (iosqs[i])->GetQId());

        SendCmd(iosqs[i], iocqs[i], writeCmd, isrCount);
        SendCmd(iosqs[i], iocqs[i], readCmd, isrCount);

        if (((iocqs[i])->GetQId() % 2) == 0) {
            if (isrCount != 4)
                throw FrmwkEx(HERE, "Invalid isrCount %d expected 4", isrCount);
        } else if (isrCount != 2) {
            throw FrmwkEx(HERE, "Invalid isrCount %d expected 2", isrCount);
        }
        VerifyData(readCmd, writeCmd);
    }
}


void
MaxIOQMSIX1To1_r10b::CreateIOQs(SharedASQPtr asq, SharedACQPtr acq,
    uint32_t ioqId, bool enableIrq, SharedIOSQPtr &iosq, SharedIOCQPtr &iocq)
{
    uint32_t numEntries = 2;

    if (Queues::SupportDiscontigIOQ() == true) {
        uint8_t iocqes = (gInformative->GetIdentifyCmdCtrlr()->
            GetValue(IDCTRLRCAP_CQES) & 0xf);
        uint8_t iosqes = (gInformative->GetIdentifyCmdCtrlr()->
            GetValue(IDCTRLRCAP_SQES) & 0xf);
        SharedMemBufferPtr iocqBackedMem = SharedMemBufferPtr(new MemBuffer());
        iocqBackedMem->InitOffset1stPage((numEntries * (1 << iocqes)), 0, true);
        iocq = Queues::CreateIOCQDiscontigToHdw(mGrpName, mTestName,
            CALC_TIMEOUT_ms(1), asq, acq, ioqId, numEntries,
            false, IOCQ_GROUP_ID, enableIrq, ioqId, iocqBackedMem);

        SharedMemBufferPtr iosqBackedMem = SharedMemBufferPtr(new MemBuffer());
        iosqBackedMem->InitOffset1stPage((numEntries * (1 << iosqes)), 0,true);
        iosq = Queues::CreateIOSQDiscontigToHdw(mGrpName, mTestName,
            CALC_TIMEOUT_ms(1), asq, acq, ioqId, numEntries, false,
            IOSQ_GROUP_ID, ioqId, 0, iosqBackedMem);
    } else {
        iocq = Queues::CreateIOCQContigToHdw(mGrpName, mTestName,
            CALC_TIMEOUT_ms(1), asq, acq, ioqId, numEntries, false,
            IOCQ_GROUP_ID, enableIrq, ioqId);
        iosq = Queues::CreateIOSQContigToHdw(mGrpName, mTestName,
            CALC_TIMEOUT_ms(1), asq, acq, ioqId, numEntries, false,
            IOSQ_GROUP_ID, ioqId, 0);
    }
}


void
MaxIOQMSIX1To1_r10b::SendCmd(SharedIOSQPtr iosq, SharedIOCQPtr iocq,
    SharedCmdPtr cmd, uint32_t &isrCount)
{
    uint16_t uniqueId;
    uint32_t numCE;
    string work;

    LOG_NRM("Send the cmd to hdw via IOSQ #%d", iosq->GetQId());
    iosq->Send(cmd, uniqueId);
    work = str(boost::format("ioqId.%d.%s") % iosq->GetQId() %
        cmd->GetName().c_str());
    iosq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "iosq", work),
        "Just B4 ringing doorbell, dump IOSQ");
    iosq->Ring();

    LOG_NRM("Wait for the CE to arrive in CQ %d", iocq->GetQId());
    if (iocq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(1), 1, numCE, isrCount)
        == false) {
        iocq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "iocq.fail"),
            "Dump Entire IOCQ");
        iosq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "iosq.fail"),
            "Dump Entire IOSQ");
        throw FrmwkEx(HERE, "Unable to see CEs for issued cmd");
    }

    work = str(boost::format("iocq.%d") % uniqueId);
    IO::ReapCE(iocq, 1, isrCount, mGrpName, mTestName, work);
}


void
MaxIOQMSIX1To1_r10b::VerifyData(SharedReadPtr readCmd,
    SharedWritePtr writeCmd)
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
