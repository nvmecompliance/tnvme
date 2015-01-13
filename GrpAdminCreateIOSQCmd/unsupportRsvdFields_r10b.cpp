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
#include "unsupportRsvdFields_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/queues.h"
#include "../Utils/io.h"

namespace GrpAdminCreateIOSQCmd {


UnsupportRsvdFields_r10b::UnsupportRsvdFields_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Set unsupported/rsvd fields in cmd");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Find 1st bare namspc, or find 1st meta namspc, or find 1st E2E "
        "namspc. Unsupported DW's and rsvd fields are treated identical, the "
        "recipient shall not check their value.  Issue a CreateIOCQ cmd with "
        "QID=1, num elements=2, and then issue a corresponding CreateIOSQ, same "
        "parameters, but set all unsupported/rsvd fields, expect success. Set: "
        "DW0_b15:10, DW2, DW3, DW4, DW5, DW6, DW7, DW11_b15:3, DW12, DW13, "
        "DW14, DW15. Issue a write cmd sending 1 block and approp supporting "
        "meta/E2E if necessary to the selected namspc at LBA 0, data pattern "
        "of word++, read back, verify pattern.");
}


UnsupportRsvdFields_r10b::~UnsupportRsvdFields_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


UnsupportRsvdFields_r10b::
UnsupportRsvdFields_r10b(const UnsupportRsvdFields_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


UnsupportRsvdFields_r10b &
UnsupportRsvdFields_r10b::operator=(const UnsupportRsvdFields_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
UnsupportRsvdFields_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    if ((preserve == false) && gCmdLine.rsvdfields)
        return RUN_TRUE;
    return RUN_FALSE;    // Optional test skipped or is destructive
}


void
UnsupportRsvdFields_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */
    uint64_t maxIOQEntries = 2;

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    LOG_NRM("Setup element sizes for the IOQ's");
    uint8_t iocqes = (gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_CQES) & 0xf);
    uint8_t iosqes = (gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_SQES) & 0xf);
    gCtrlrConfig->SetIOCQES(iocqes);
    gCtrlrConfig->SetIOSQES(iosqes);

    LOG_NRM("Create IOCQ");
    SharedIOCQPtr iocq = Queues::CreateIOCQContigToHdw(mGrpName, mTestName,
        CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, maxIOQEntries, false,
        IOCQ_GROUP_ID, true, 0);

    LOG_NRM("Create IOSQ");
    SharedIOSQPtr iosq = SharedIOSQPtr(new IOSQ(gDutFd));

    LOG_NRM("Allocate contiguous memory; IOSQ has ID=%d", IOQ_ID);
    iosq->Init(IOQ_ID, maxIOQEntries, IOQ_ID, 0);

    LOG_NRM("Form a Create IOSQ cmd to perform queue creation");
    SharedCreateIOSQPtr createIOSQCmd = SharedCreateIOSQPtr(new CreateIOSQ());
    createIOSQCmd->Init(iosq);

    LOG_NRM("Set all cmd's rsvd bits");
    uint32_t work = createIOSQCmd->GetDword(0);
    work |= 0x0000fc00;      // Set DW0_b15:10 bits
    createIOSQCmd->SetDword(work, 0);

    createIOSQCmd->SetDword(0xffffffff, 2);
    createIOSQCmd->SetDword(0xffffffff, 3);
    createIOSQCmd->SetDword(0xffffffff, 4);
    createIOSQCmd->SetDword(0xffffffff, 5);
    createIOSQCmd->SetDword(0xffffffff, 6);
    createIOSQCmd->SetDword(0xffffffff, 7);

    // DW11_b15:3
    work = createIOSQCmd->GetDword(11);
    work |= 0x0000fff8;
    createIOSQCmd->SetDword(work, 11);

    createIOSQCmd->SetDword(0xffffffff, 12);
    createIOSQCmd->SetDword(0xffffffff, 13);
    createIOSQCmd->SetDword(0xffffffff, 14);
    createIOSQCmd->SetDword(0xffffffff, 15);

    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
        createIOSQCmd, "", true);

    WriteReadVerify(iosq, iocq);

    Queues::DeleteIOSQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq,
        asq, acq, "", false);
    Queues::DeleteIOCQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iocq,
        asq, acq, "", false);
}


void
UnsupportRsvdFields_r10b::WriteReadVerify(SharedIOSQPtr iosq,
    SharedIOCQPtr iocq)
{
    Informative::Namspc namspcData = gInformative->Get1stBareMetaE2E();
    LBAFormat lbaFormat = namspcData.idCmdNamspc->GetLBAFormat();
    uint64_t lbaDataSize = namspcData.idCmdNamspc->GetLBADataSize();

    SharedWritePtr writeCmd = SharedWritePtr(new Write());
    SharedMemBufferPtr writeMem = SharedMemBufferPtr(new MemBuffer());

    SharedReadPtr readCmd = SharedReadPtr(new Read());
    SharedMemBufferPtr readMem = SharedMemBufferPtr(new MemBuffer());

    send_64b_bitmask prpBitmask = (send_64b_bitmask)
        (MASK_PRP1_PAGE | MASK_PRP2_PAGE | MASK_PRP2_LIST);

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

    switch (namspcData.type) {
    case Informative::NS_BARE:
        writeMem->SetDataPattern(DATAPAT_INC_32BIT);
        break;
    case Informative::NS_METAS:
        writeMem->SetDataPattern(DATAPAT_INC_32BIT);
        writeCmd->SetMetaDataPattern(DATAPAT_INC_32BIT);
        break;
    case Informative::NS_METAI:
        writeMem->SetDataPattern(DATAPAT_INC_32BIT);
        break;
    case Informative::NS_E2ES:
    case Informative::NS_E2EI:
        throw FrmwkEx(HERE, "Deferring work to handle this case in future");
        break;
    }

    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq,
        iocq, writeCmd, "", true);

    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq,
        iocq, readCmd, "", true);

    VerifyDataPat(readCmd, writeCmd);
}


void
UnsupportRsvdFields_r10b::VerifyDataPat(SharedReadPtr readCmd,
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

