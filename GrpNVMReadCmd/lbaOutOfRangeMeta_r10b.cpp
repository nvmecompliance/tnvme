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
#include "lbaOutOfRangeMeta_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/acq.h"
#include "../Queues/asq.h"
#include "../Utils/io.h"
#include "../Utils/irq.h"
#include "../Cmds/read.h"

namespace GrpNVMReadCmd {

#define RD_NUM_BLKS                 2


LBAOutOfRangeMeta_r10b::LBAOutOfRangeMeta_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4,6");
    mTestDesc.SetShort(     "Issue read and cause SC = LBA Out of Range on meta namspcs");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "For all meta namspcs from Identify.NN, determine Identify.NSZE; "
        "For each namspc cause many scenarios by issuing a single read cmd "
        "requesting 2 data blocks, and conforming to approp metadata "
        "requirements. 1) Issue cmd where 1st block starts at LBA "
        "(Identify.NSZE - 1), expect failure. 2) Issue cmd where 1st block "
        "starts at LBA Identify.NSZE, expect failure. 3) Issue cmd where 1st "
        "block starts at 2nd to last max LBA value, expect success.");
}


LBAOutOfRangeMeta_r10b::~LBAOutOfRangeMeta_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


LBAOutOfRangeMeta_r10b::
LBAOutOfRangeMeta_r10b(const LBAOutOfRangeMeta_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


LBAOutOfRangeMeta_r10b &
LBAOutOfRangeMeta_r10b::operator=(const LBAOutOfRangeMeta_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
LBAOutOfRangeMeta_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    preserve = preserve;    // Suppress compiler error/warning
    return RUN_TRUE;        // This test is never destructive
}


void
LBAOutOfRangeMeta_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * None.
     * \endverbatim
     */
    uint64_t nsze;
    string work;
    ConstSharedIdentifyPtr namSpcPtr;
    SharedIOSQPtr iosq;
    SharedIOCQPtr iocq;


    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    SharedACQPtr acq = SharedACQPtr(new ACQ(gDutFd));
    acq->Init(5);

    SharedASQPtr asq = SharedASQPtr(new ASQ(gDutFd));
    asq->Init(5);

    vector<uint32_t> meta = gInformative->GetMetaNamespaces();
    for (size_t i = 0; i < meta.size(); i++) {
        if (gCtrlrConfig->SetState(ST_DISABLE) == false)
            throw FrmwkEx(HERE);

        namSpcPtr = gInformative->GetIdentifyCmdNamspc(meta[i]);
        nsze = namSpcPtr->GetValue(IDNAMESPC_NSZE);
        LBAFormat lbaFormat = namSpcPtr->GetLBAFormat();

        // All queues will use identical IRQ vector
        IRQ::SetAnySchemeSpecifyNum(1);

        gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
        if (gCtrlrConfig->SetState(ST_ENABLE) == false)
            throw FrmwkEx(HERE);

        LOG_NRM("Create IOSQ and IOCQ with ID #%d", IOQ_ID);
        CreateIOQs(asq, acq, IOQ_ID, iosq, iocq);

        LOG_NRM("Create memory to contain read payload");
        SharedMemBufferPtr readMem = SharedMemBufferPtr(new MemBuffer());
        uint64_t lbaDataSize = (1 << lbaFormat.LBADS);

        LOG_NRM("Create a read cmd to read data from namspc %d", meta[i]);
        SharedReadPtr readCmd = SharedReadPtr(new Read());
        send_64b_bitmask prpBitmask = (send_64b_bitmask)
            (MASK_PRP1_PAGE | MASK_PRP2_PAGE | MASK_PRP2_LIST);

        LOG_NRM("Determine namspc type and allocate meta buffer as necesssary");
        Informative::NamspcType nsType =
            gInformative->IdentifyNamespace(namSpcPtr);;
        switch (nsType) {
        case Informative::NS_BARE:
            throw FrmwkEx(HERE, "Namspc type cannot be BARE.");
        case Informative::NS_METAS:
            LOG_NRM("Meta mamespace with separate buffer. meta size = %d",
                lbaFormat.MS);
            readMem->Init(RD_NUM_BLKS * lbaDataSize);
            if (gRsrcMngr->SetMetaAllocSize(RD_NUM_BLKS * lbaFormat.MS)
                == false) {
                throw FrmwkEx(HERE);
            }
            readCmd->AllocMetaBuffer();
            break;
        case Informative::NS_METAI:
            LOG_NRM("Meta mamespace with extended LBA size = %ld.",
                (lbaDataSize + lbaFormat.MS));
            readMem->Init(RD_NUM_BLKS * (lbaDataSize + lbaFormat.MS));
            break;
        case Informative::NS_E2ES:
        case Informative::NS_E2EI:
            throw FrmwkEx(HERE, "Deferring work to handle this case in future");
            break;
        }
        LOG_NRM("Set read cmd options after buffer is allocated.");
        readCmd->SetPrpBuffer(prpBitmask, readMem);
        readCmd->SetNSID(meta[i]);
        readCmd->SetNLB(RD_NUM_BLKS - 1);    // convert to 0-based value

        LOG_NRM("Issue cmd where 1st block starts at LBA (Identify.NSZE - 1)");
        work = str(boost::format("nsze-1.meta.%d") % (uint32_t)i);
        readCmd->SetSLBA(nsze - 1);
        SendCmdToHdw(iosq, iocq, readCmd, work);

        LOG_NRM("Issue cmd where 1st block starts at LBA (Identify.NSZE)");
        work = str(boost::format("nsze.meta.%d") % (uint32_t)i);
        readCmd->SetSLBA(nsze);
        SendCmdToHdw(iosq, iocq, readCmd, work);

        LOG_NRM("Issue cmd where 1st block starts at LBA (Identify.NSZE - 2)");
        work = str(boost::format("nsze-2.meta.%d") % (uint32_t)i);
        readCmd->SetSLBA(nsze - 2);
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq,
            iocq, readCmd, work, true);
    }
}


void
LBAOutOfRangeMeta_r10b::SendCmdToHdw(SharedSQPtr sq, SharedCQPtr cq,
    SharedCmdPtr cmd, string qualify)
{
    uint32_t numCE;
    uint32_t isrCount;
    uint32_t isrCountB4;
    string work;
    uint16_t uniqueId;

    if ((numCE = cq->ReapInquiry(isrCountB4, true)) != 0) {
        cq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName, "cq",
            "notEmpty"), "Test assumption have not been met");
        throw FrmwkEx(HERE, "Require 0 CE's within CQ %d, not upheld, found %d",
            cq->GetQId(), numCE);
    }

    LOG_NRM("Send the cmd to hdw via SQ %d", sq->GetQId());
    sq->Send(cmd, uniqueId);
    work = str(boost::format(
        "Just B4 ringing SQ %d doorbell, dump entire SQ") % sq->GetQId());
    sq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName,
        "sq." + cmd->GetName(), qualify), work);
    sq->Ring();

    LOG_NRM("Wait for the CE to arrive in CQ %d", cq->GetQId());
    if (cq->ReapInquiryWaitSpecify(CALC_TIMEOUT_ms(1), 1, numCE, isrCount)
        == false) {
        work = str(boost::format(
            "Unable to see any CE's in CQ %d, dump entire CQ") % cq->GetQId());
        cq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName,
            "cq." + cmd->GetName(), qualify), work);
        throw FrmwkEx(HERE, "Unable to see CE for issued cmd");
    } else if (numCE != 1) {
        work = str(boost::format(
            "Unable to see any CE's in CQ %d, dump entire CQ") % cq->GetQId());
        cq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName,
            "cq." + cmd->GetName(), qualify), work);
        throw FrmwkEx(HERE, "1 cmd caused %d CE's to arrive in CQ %d",
            numCE, cq->GetQId());
    }
    work = str(boost::format("Just B4 reaping CQ %d, dump entire CQ") %
        cq->GetQId());
    cq->Dump(FileSystem::PrepDumpFile(mGrpName, mTestName,
        "cq." + cmd->GetName(), qualify), work);

    // throws if an error occurs
    IO::ReapCE(cq, numCE, isrCount, mGrpName, mTestName, qualify,
        CESTAT_LBA_OUT_RANGE);

    // Single cmd submitted on empty ASQ should always yield 1 IRQ on ACQ
    if (gCtrlrConfig->IrqsEnabled() && cq->GetIrqEnabled() &&
        (isrCount != (isrCountB4 + 1))) {
        throw FrmwkEx(HERE,
            "CQ using IRQ's, but IRQ count not expected (%d != %d)",
            isrCount, (isrCountB4 + 1));
    }
}



void
LBAOutOfRangeMeta_r10b::CreateIOQs(SharedASQPtr asq, SharedACQPtr acq,
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

