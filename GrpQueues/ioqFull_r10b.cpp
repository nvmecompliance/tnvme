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

#include "boost/format.hpp"
#include "ioqFull_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/io.h"

namespace GrpQueues {


IOQFull_r10b::IOQFull_r10b(int fd, string grpName,
    string testName, ErrorRegs errRegs) :
    Test(fd, grpName, testName, SPECREV_10b, errRegs)
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


void
IOQFull_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     *  \endverbatim
     */

    uint64_t ctrlCapReg;
    // Determine the max IOQ entries supported
    if (gRegisters->Read(CTLSPC_CAP, ctrlCapReg) == false) {
        LOG_ERR("Unable to determine MQES");
        throw FrmwkEx();
    }
    uint16_t maxIOQEntries = (uint16_t)(ctrlCapReg & CAP_MQES);

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    SharedWritePtr writeCmd = SetWriteCmd();

    // Case 1 - IOSQ = IOCQ (min, middle and max)
    IOQFull(2, 2, asq, acq, writeCmd);
    IOQFull((maxIOQEntries/2), (maxIOQEntries/2), asq, acq, writeCmd);
    IOQFull(maxIOQEntries, maxIOQEntries, asq, acq, writeCmd);

    // Case 2 - IOSQ =  IOCQ + 1 (min , middle and max)
    IOQFull(3, 2, asq, acq, writeCmd);
    IOQFull((maxIOQEntries/2), ((maxIOQEntries/2) - 1), asq, acq, writeCmd);
    IOQFull(maxIOQEntries, (maxIOQEntries - 1), asq, acq, writeCmd);

}


void
IOQFull_r10b::IOQFull(uint16_t numIOSQEntries, uint16_t numIOCQEntries,
        SharedASQPtr asq, SharedACQPtr acq, SharedWritePtr writeCmd)
{
    string work;
    uint16_t numCE;
    uint32_t isrCount;

    // Reset the controller.
    if (gCtrlrConfig->SetState(ST_DISABLE) == false)
        throw FrmwkEx();
    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx();

    uint8_t iocqes = (gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_CQES) & 0xf);
    gCtrlrConfig->SetIOCQES(iocqes);
    SharedMemBufferPtr iocqBackedMem = SharedMemBufferPtr(new MemBuffer());
    iocqBackedMem->InitOffset1stPage((numIOCQEntries * (1 << iocqes)), 0, true);
    SharedIOCQPtr iocq = Queues::CreateIOCQDiscontigToHdw(mGrpName,
        mTestName, DEFAULT_CMD_WAIT_ms, asq, acq, IOQ_ID, numIOCQEntries,
        false, IOCQ_CONTIG_GROUP_ID, false, 1, iocqBackedMem);

    uint8_t iosqes = (gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_SQES) & 0xf);
    gCtrlrConfig->SetIOSQES(iosqes);
    SharedMemBufferPtr iosqBackedMem = SharedMemBufferPtr(new MemBuffer());
    iosqBackedMem->InitOffset1stPage((numIOSQEntries * (1 << iosqes)), 0, true);
    SharedIOSQPtr iosq = Queues::CreateIOSQDiscontigToHdw(mGrpName,
        mTestName, DEFAULT_CMD_WAIT_ms, asq, acq, IOQ_ID, numIOSQEntries,
        false, IOSQ_CONTIG_GROUP_ID, IOQ_ID, 0, iosqBackedMem);

    uint16_t nCmdsToSubmit = numIOSQEntries - 1;
    LOG_NRM("Send #%d cmds to hdw via IOSQ", nCmdsToSubmit);
    for (uint16_t nCmds = 0; nCmds < nCmdsToSubmit; nCmds++) {
        LOG_NRM("Sending #%d of #%d NVM Write Cmds thru IOSQ", nCmds + 1,
            nCmdsToSubmit);
        iosq->Send(writeCmd);
        iosq->Ring();

        LOG_NRM("Wait for the CE to arrive in IOCQ");
        if (iocq->ReapInquiryWaitSpecify(DEFAULT_CMD_WAIT_ms, (nCmds + 1),
            numCE, isrCount) == false) {
            // if iosq size equals (iocq size + 1), last CE will not arrive.
            if ((numIOSQEntries == numIOCQEntries + 1) &&
                (nCmds == nCmdsToSubmit - 1)) {
                // Reap one element from IOCQ to make room for last CE.
                IO::ReapCE(iocq, 1, isrCount, mGrpName, mTestName, "IOCQCE",
                    CESTAT_SUCCESS);
                if (iocq->ReapInquiryWaitSpecify(DEFAULT_CMD_WAIT_ms, nCmds,
                    numCE, isrCount) == false) {
                    work = str(boost::format("Dump entire CQ %d") %
                        iocq->GetQId());
                    iocq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName,
                        "iocq." + writeCmd->GetName()), work);
                    throw FrmwkEx("Unable to see last CE as expected");
                }
                break;
            }
            work = str(boost::format("Dump entire CQ %d") % iocq->GetQId());
            iocq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName,
                "iocq." + writeCmd->GetName()), work);
            throw FrmwkEx("Unable to see CE for issued cmd #%d", nCmds + 1);
        } else if (numCE != nCmds + 1) {
            work = str(boost::format("Dump entire CQ %d") % iocq->GetQId());
            iocq->Dump(FileSystem::PrepLogFile(mGrpName, mTestName,
                "iocq." + writeCmd->GetName()), work);
            throw FrmwkEx("Missing last CE, #%d cmds of #%d received",
                nCmds + 1, numCE);
        }
    }

}


SharedWritePtr
IOQFull_r10b::SetWriteCmd()
{
    Informative::Namspc namspcData = gInformative->Get1stBareMetaE2E();
    LOG_NRM("Processing write cmd using namspc id %d", namspcData.id);
    if (namspcData.type != Informative::NS_BARE) {
        LBAFormat lbaFormat = namspcData.idCmdNamspc->GetLBAFormat();
        if (gRsrcMngr->SetMetaAllocSize(lbaFormat.MS) == false)
            throw FrmwkEx();
    }

    LOG_NRM("Create data pattern to write to media");
    SharedMemBufferPtr dataPat = SharedMemBufferPtr(new MemBuffer());
    uint64_t lbaDataSize = namspcData.idCmdNamspc->GetLBADataSize();
    dataPat->Init(lbaDataSize);

    SharedWritePtr writeCmd = SharedWritePtr(new Write());
    send_64b_bitmask prpBitmask = (send_64b_bitmask)(MASK_PRP1_PAGE
        | MASK_PRP2_PAGE | MASK_PRP2_LIST);

    if (namspcData.type == Informative::NS_META) {
        writeCmd->AllocMetaBuffer();
        prpBitmask = (send_64b_bitmask)(prpBitmask | MASK_MPTR);
    } else if (namspcData.type == Informative::NS_E2E) {
        writeCmd->AllocMetaBuffer();
        prpBitmask = (send_64b_bitmask)(prpBitmask | MASK_MPTR);
        LOG_ERR("Deferring E2E namspc work to the future");
        LOG_ERR("Need to add CRC's to correlate to buf pattern");
        throw FrmwkEx();
    }

    writeCmd->SetPrpBuffer(prpBitmask, dataPat);
    writeCmd->SetNSID(namspcData.id);
    writeCmd->SetNLB(0);

    return writeCmd;
}

}   // namespace
