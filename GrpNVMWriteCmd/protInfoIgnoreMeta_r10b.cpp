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
#include "protInfoIgnoreMeta_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/io.h"
#include "../Utils/irq.h"
#include "../Cmds/write.h"

namespace GrpNVMWriteCmd {


ProtInfoIgnoreMeta_r10b::ProtInfoIgnoreMeta_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Verify protection info (PRINFO) is ignored for meta namspc");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "For all meta namspcs from Identify.NN; For each namspc issue multiple "
        "write cmds where each is sending 1 data block at LBA 0 and approp "
        "metadata requirements, and vary the values of DW12.PRINFO "
        "from 0x0 to 0x0f, expect success for all.");
}


ProtInfoIgnoreMeta_r10b::~ProtInfoIgnoreMeta_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


ProtInfoIgnoreMeta_r10b::
ProtInfoIgnoreMeta_r10b(const ProtInfoIgnoreMeta_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


ProtInfoIgnoreMeta_r10b &
ProtInfoIgnoreMeta_r10b::operator=(const ProtInfoIgnoreMeta_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
ProtInfoIgnoreMeta_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
ProtInfoIgnoreMeta_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * None.
     * \endverbatim
     */
    string context;
    ConstSharedIdentifyPtr namSpcPtr;
    SharedIOSQPtr iosq;
    SharedIOCQPtr iocq;
    send_64b_bitmask prpBitmask = (send_64b_bitmask)
        (MASK_PRP1_PAGE | MASK_PRP2_PAGE | MASK_PRP2_LIST);

    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    SharedACQPtr acq = SharedACQPtr(new ACQ(gDutFd));
    acq->Init(5);

    SharedASQPtr asq = SharedASQPtr(new ASQ(gDutFd));
    asq->Init(5);

    LOG_NRM("Get all the supoorted meta namespaces");
    vector<uint32_t> meta = gInformative->GetMetaNamespaces();
    for (size_t i = 0; i < meta.size(); i++) {
        if (gCtrlrConfig->SetState(ST_DISABLE) == false)
            throw FrmwkEx(HERE);

        // All queues will use identical IRQ vector
        IRQ::SetAnySchemeSpecifyNum(1);

        gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
        if (gCtrlrConfig->SetState(ST_ENABLE) == false)
            throw FrmwkEx(HERE);

        LOG_NRM("Create IOSQ and IOCQ with ID #%d", IOQ_ID);
        CreateIOQs(asq, acq, IOQ_ID, iosq, iocq);

        LOG_NRM("Get LBA format and lba data size for namespc #%d", meta[i]);
        namSpcPtr = gInformative->GetIdentifyCmdNamspc(meta[i]);
        LBAFormat lbaFormat = namSpcPtr->GetLBAFormat();
        uint64_t lbaDataSize = (1 << lbaFormat.LBADS);

        LOG_NRM("Create a write cmd to write data to namspc %d", meta[i]);
        SharedWritePtr writeCmd = SharedWritePtr(new Write());

        LOG_NRM("Create memory to contain write payload");
        SharedMemBufferPtr writeMem = SharedMemBufferPtr(new MemBuffer());

        Informative::NamspcType
            nsType = gInformative->IdentifyNamespace(namSpcPtr);
        switch (nsType) {
        case Informative::NS_BARE:
            throw FrmwkEx(HERE, "Namspc type cannot be BARE.");
        case Informative::NS_METAS:
            writeMem->Init(lbaDataSize);
            if (gRsrcMngr->SetMetaAllocSize(lbaFormat.MS) == false)
                throw FrmwkEx(HERE);
            writeCmd->AllocMetaBuffer();
            break;
        case Informative::NS_METAI:
            writeMem->Init(lbaDataSize + lbaFormat.MS);
            break;
        case Informative::NS_E2ES:
        case Informative::NS_E2EI:
            throw FrmwkEx(HERE, "Deferring work to handle this case in future");
            break;
        }

        writeCmd->SetPrpBuffer(prpBitmask, writeMem);
        writeCmd->SetNSID(meta[i]);
        writeCmd->SetNLB(0);    // convert to 0-based value

        for (uint16_t protInfo = 0; protInfo <= 0x0f; protInfo++) {
            uint8_t work = writeCmd->GetByte(12, 3);
            work &= ~0x3c;  // PRINFO specific bits
            work |= (protInfo << 2);
            writeCmd->SetByte(work, 12, 3);

            context = str(boost::format("ns%d.protInfo0x%02X") %
                (uint32_t)i % protInfo);
            IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq,
                iocq, writeCmd, context, true);
        }
    }
}


void
ProtInfoIgnoreMeta_r10b::CreateIOQs(SharedASQPtr asq, SharedACQPtr acq,
    uint32_t ioqId, SharedIOSQPtr &iosq, SharedIOCQPtr &iocq)
{
    uint32_t numEntries = 2;

    gCtrlrConfig->SetIOCQES((gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_CQES) & 0xf));
    gCtrlrConfig->SetIOSQES((gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_SQES) & 0xf));

    if (Queues::SupportDiscontigIOQ() == true) {
        uint8_t iocqes = (gInformative->GetIdentifyCmdCtrlr()->
            GetValue(IDCTRLRCAP_CQES) & 0xf);
        uint8_t iosqes = (gInformative->GetIdentifyCmdCtrlr()->
            GetValue(IDCTRLRCAP_SQES) & 0xf);
        SharedMemBufferPtr iocqBackedMem = SharedMemBufferPtr(new MemBuffer());
        iocqBackedMem->InitOffset1stPage((numEntries * (1 << iocqes)), 0, true);
        iocq = Queues::CreateIOCQDiscontigToHdw(mGrpName, mTestName,
            CALC_TIMEOUT_ms(1), asq, acq, ioqId, numEntries,
            false, IOCQ_GROUP_ID, true, 0, iocqBackedMem);

        SharedMemBufferPtr iosqBackedMem = SharedMemBufferPtr(new MemBuffer());
        iosqBackedMem->InitOffset1stPage((numEntries * (1 << iosqes)), 0,true);
        iosq = Queues::CreateIOSQDiscontigToHdw(mGrpName, mTestName,
            CALC_TIMEOUT_ms(1), asq, acq, ioqId, numEntries, false,
            IOSQ_GROUP_ID, ioqId, 0, iosqBackedMem);
    } else {
        iocq = Queues::CreateIOCQContigToHdw(mGrpName, mTestName,
            CALC_TIMEOUT_ms(1), asq, acq, ioqId, numEntries, false,
            IOCQ_GROUP_ID, true, 0);
        iosq = Queues::CreateIOSQContigToHdw(mGrpName, mTestName,
            CALC_TIMEOUT_ms(1), asq, acq, ioqId, numEntries, false,
            IOSQ_GROUP_ID, ioqId, 0);
    }
}


}   // namespace
