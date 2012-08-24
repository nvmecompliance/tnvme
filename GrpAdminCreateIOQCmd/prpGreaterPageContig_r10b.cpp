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
#include "prpGreaterPageContig_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/irq.h"
#include "../Utils/io.h"
#include "../Utils/queues.h"
#include "../Queues/acq.h"
#include "../Queues/asq.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/kernelAPI.h"


namespace GrpAdminCreateIOQCmd {


PRPGreaterPageContig_r10b::PRPGreaterPageContig_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Create IOQ's backed by contiguous memory > 1 page");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "May only run this test if ((CAP.MQES + 1) >= Y. Search for 1 of the "
        "following namspcs to run test. Find 1st bare namspc, or find 1st "
        "meta namspc, or find 1st E2E namspc. Issue a CreateIOCQ cmd, with "
        "QID = 1, num elements = Y, where Y = ((CC.MPS / CC.IOCQES) + 1), "
        "where CC.IOCQES = Identify.CQES_b3:0; backed by contig memory. "
        "Issue a CreateIOSQ cmd, with QID = 1, num elements = Z, where "
        "Z = ((CC.MPS / CC.IOSQES) + 1), where CC.IOSQES = Identify.SQES_b3:0; "
        "backed by contig memory. Issue X write cmds, where X = "
        "(num elements in IOSQ + 1), sending 1 block and approp supporting "
        "meta/E2E if necessary to the selected namspc at LBA 0, data pattern "
        "of word++, read back, verify pattern.");
}


PRPGreaterPageContig_r10b::~PRPGreaterPageContig_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


PRPGreaterPageContig_r10b::
PRPGreaterPageContig_r10b(const PRPGreaterPageContig_r10b &other) :
    Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


PRPGreaterPageContig_r10b &
PRPGreaterPageContig_r10b::operator=(const PRPGreaterPageContig_r10b
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
PRPGreaterPageContig_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
PRPGreaterPageContig_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * None
     * \endverbatim
     */
    string work;
    bool enableLog;

    uint64_t ctrlCapReg;
    LOG_NRM("Determine the max IOQ entries supported");
    if (gRegisters->Read(CTLSPC_CAP, ctrlCapReg) == false)
        throw FrmwkEx(HERE, "Unable to determine MQES");
    uint32_t maxIOQEntries = (ctrlCapReg & CAP_MQES);
    maxIOQEntries += 1;      // convert to 1-based

    LOG_NRM("Compute memory page size from CC.MPS.");
    uint8_t mps;
    if (gCtrlrConfig->GetMPS(mps) == false)
        throw FrmwkEx(HERE, "Unable to get MPS value from CC.");
    uint64_t capMPS = (uint64_t)(1 << (mps + 12));

    LOG_NRM("Determine element sizes for the IOCQ's");
    uint8_t iocqes = (gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_CQES) & 0xf);
    uint32_t Y = ((capMPS / (1 << iocqes)) + 1);
    if (maxIOQEntries < Y) {
        LOG_WARN("Desired to support >= %d elements in IOCQ for this test", Y);
        return;
    }

    LOG_NRM("Determine element sizes for the IOSQ's");
    uint8_t iosqes = (gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_SQES) & 0xf);
    uint32_t Z = ((capMPS / (1 << iosqes)) + 1);

    LOG_NRM("Computed memory page size from CC.MPS = %ld", capMPS);
    LOG_NRM("Max IOQ entries supported CAP.MQES = %d", maxIOQEntries);
    LOG_NRM("Number of IOCQ elements = %d", Y);
    LOG_NRM("Number of IOSQ elements = %d", Z);

    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Create admin queues ACQ and ASQ");
    SharedACQPtr acq = SharedACQPtr(new ACQ(gDutFd));
    acq->Init(5);

    SharedASQPtr asq = SharedASQPtr(new ASQ(gDutFd));
    asq->Init(5);

    // All queues will use identical IRQ vector
    IRQ::SetAnySchemeSpecifyNum(1);     // throws upon error

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);

    Informative::Namspc namspcData = gInformative->Get1stBareMetaE2E();
    LBAFormat lbaFormat = namspcData.idCmdNamspc->GetLBAFormat();
    uint64_t lbaDataSize = namspcData.idCmdNamspc->GetLBADataSize();

    LOG_NRM("Setup element sizes for the IOQ's");
    gCtrlrConfig->SetIOCQES(iocqes);
    gCtrlrConfig->SetIOSQES(iosqes);

    SharedIOCQPtr iocq = Queues::CreateIOCQContigToHdw(mGrpName, mTestName,
        CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, Y, true,
        IOCQ_GROUP_ID, true, 0);

    SharedIOSQPtr iosq = Queues::CreateIOSQContigToHdw(mGrpName, mTestName,
        CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, Z, true,
        IOSQ_GROUP_ID, IOQ_ID, 0);

    SharedWritePtr writeCmd = SharedWritePtr(new Write());
    SharedMemBufferPtr writeMem = SharedMemBufferPtr(new MemBuffer());

    SharedReadPtr readCmd = SharedReadPtr(new Read());
    SharedMemBufferPtr readMem = SharedMemBufferPtr(new MemBuffer());

    send_64b_bitmask prpBitmask = (send_64b_bitmask)(MASK_PRP1_PAGE |
        MASK_PRP2_PAGE | MASK_PRP2_LIST);

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

    // Fill the IOSQ and roll over
    for (int64_t X = 1; X <= (iosq->GetNumEntries() + 1); X++) {
        LOG_NRM("Processing #%ld of %d cmds", X, (iosq->GetNumEntries() + 1));
        switch (namspcData.type) {
        case Informative::NS_BARE:
            writeMem->SetDataPattern(DATAPAT_INC_32BIT, X);
            break;
        case Informative::NS_METAS:
            writeMem->SetDataPattern(DATAPAT_INC_32BIT, X);
            writeCmd->SetMetaDataPattern(DATAPAT_INC_32BIT, X);
            break;
        case Informative::NS_METAI:
            writeMem->SetDataPattern(DATAPAT_INC_32BIT, X);
            break;
        case Informative::NS_E2ES:
        case Informative::NS_E2EI:
            throw FrmwkEx(HERE, "Deferring work to handle this case in future");
            break;
        }

        enableLog = false;
        if ((X <= 8) || (X >= (X - 8)))
            enableLog = true;

        work = str(boost::format("X.%d") % X);
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq,
            iocq, writeCmd, work, enableLog);

        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq,
            iocq, readCmd, work, enableLog);

        VerifyDataPat(readCmd, writeCmd);
    }
}


void
PRPGreaterPageContig_r10b::VerifyDataPat(SharedReadPtr readCmd,
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
