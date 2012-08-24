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
#include "startingLBAMeta_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/io.h"
#include "../Utils/irq.h"


namespace GrpNVMWriteReadCombo {


StartingLBAMeta_r10b::StartingLBAMeta_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Vary starting LBA for meta namspcs");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "For all meta namspcs from Identify.NN, determine Identify.NCAP; "
        "For each namspc issue multiple write cmds each sending "
        "Identify.MDTS, or 256 KB if unlimited, amount of data starting from "
        "LBA 0 to (Identify.NCAP - 1). Each block of data should use a new "
        "data pattern by rolling through {byte++, byteK, word++, wordK, "
        "dword++, dwordK}. After all writing completes issue correlating "
        "read cmds through the same range verifying the data pattern upon "
        "each block.");
}


StartingLBAMeta_r10b::~StartingLBAMeta_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


StartingLBAMeta_r10b::
StartingLBAMeta_r10b(const StartingLBAMeta_r10b &other) :
    Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


StartingLBAMeta_r10b &
StartingLBAMeta_r10b::operator=(const StartingLBAMeta_r10b
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
StartingLBAMeta_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
StartingLBAMeta_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * None.
     * \endverbatim
     */
    string work;
    bool enableLog;
    ConstSharedIdentifyPtr namSpcPtr;
    SharedIOSQPtr iosq;
    SharedIOCQPtr iocq;
    uint64_t maxWrBlks;

    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    SharedACQPtr acq = SharedACQPtr(new ACQ(gDutFd));
    acq->Init(5);

    SharedASQPtr asq = SharedASQPtr(new ASQ(gDutFd));
    asq->Init(5);

    ConstSharedIdentifyPtr idCmdCtrlr = gInformative->GetIdentifyCmdCtrlr();
    uint32_t maxDtXferSz = idCmdCtrlr->GetMaxDataXferSize();
    if (maxDtXferSz == 0)
        maxDtXferSz = MAX_DATA_TX_SIZE;

    LOG_NRM("Prepare cmds to be send through Q's.");
    SharedWritePtr writeCmd = SharedWritePtr(new Write());
    SharedMemBufferPtr writeMem = SharedMemBufferPtr(new MemBuffer());

    SharedReadPtr readCmd = SharedReadPtr(new Read());
    SharedMemBufferPtr readMem = SharedMemBufferPtr(new MemBuffer());

    DataPattern dataPat[] = {
        DATAPAT_INC_8BIT,
        DATAPAT_CONST_8BIT,
        DATAPAT_INC_16BIT,
        DATAPAT_CONST_16BIT,
        DATAPAT_INC_32BIT,
        DATAPAT_CONST_32BIT
    };
    uint64_t dpArrSize = sizeof(dataPat) / sizeof(dataPat[0]);
    send_64b_bitmask prpBitmask = (send_64b_bitmask)
        (MASK_PRP1_PAGE | MASK_PRP2_PAGE | MASK_PRP2_LIST);

    LOG_NRM("Seeking all meta namspc's.");
    vector<uint32_t> meta = gInformative->GetMetaNamespaces();
    for (size_t i = 0; i < meta.size(); i++) {
        LOG_NRM("Processing meta namspc id #%d of %ld", meta[i], meta.size());
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
        uint64_t ncap = namSpcPtr->GetValue(IDNAMESPC_NCAP);
        uint64_t metaBuffSz = 0;

        LOG_NRM("Set read and write buffers based on the namspc type");
        switch (gInformative->IdentifyNamespace(namSpcPtr)) {
        case Informative::NS_BARE:
            throw FrmwkEx(HERE, "Namspc type cannot be BARE.");
        case Informative::NS_METAS:
            maxWrBlks = maxDtXferSz / lbaDataSize;
            metaBuffSz = maxWrBlks * lbaFormat.MS;
            if (gRsrcMngr->SetMetaAllocSize(metaBuffSz) == false)
                throw FrmwkEx(HERE);
            LOG_NRM("Max rd/wr blks %ld using separate meta buff of ncap %ld",
                maxWrBlks, ncap);
            writeMem->Init(maxWrBlks * lbaDataSize);
            readMem->Init(maxWrBlks * lbaDataSize);
            writeCmd->AllocMetaBuffer();
            readCmd->AllocMetaBuffer();
            break;
        case Informative::NS_METAI:
            maxWrBlks = maxDtXferSz / (lbaDataSize + lbaFormat.MS);
            LOG_NRM("Max rd/wr blks %ld using integrated meta buff of ncap %ld",
                maxWrBlks, ncap);
            writeMem->Init(maxWrBlks * (lbaDataSize + lbaFormat.MS));
            readMem->Init(maxWrBlks * (lbaDataSize + lbaFormat.MS));
            break;
        case Informative::NS_E2ES:
        case Informative::NS_E2EI:
            throw FrmwkEx(HERE, "Deferring work to handle this case in future");
            break;
        }

        writeCmd->SetPrpBuffer(prpBitmask, writeMem);
        writeCmd->SetNSID(meta[i]);
        writeCmd->SetNLB(maxWrBlks - 1);  // 0 based value.

        readCmd->SetPrpBuffer(prpBitmask, readMem);
        readCmd->SetNSID(meta[i]);
        readCmd->SetNLB(maxWrBlks - 1);  // 0 based value.

        for (uint64_t sLBA = 0; sLBA < (ncap - 1); sLBA += maxWrBlks) {
            LOG_NRM("Processing at #%ld blk of %ld", sLBA, (ncap -1));
            if ((sLBA + maxWrBlks) >= ncap) {
                maxWrBlks = ncap - sLBA;
                LOG_NRM("Resize max write blocks to #%ld", maxWrBlks);
                ResizeDataBuf(readCmd, writeCmd, namSpcPtr, maxWrBlks,
                    prpBitmask);
                metaBuffSz = maxWrBlks * lbaFormat.MS;
            }
            LOG_NRM("Sending #%ld blks starting at #%ld", maxWrBlks, sLBA);
            for (uint64_t nLBA = 0; nLBA < maxWrBlks; nLBA++) {
                writeMem->SetDataPattern(dataPat[nLBA % dpArrSize],
                    (sLBA + nLBA + 1), (nLBA * lbaDataSize), lbaDataSize);
                writeCmd->SetMetaDataPattern(dataPat[nLBA % dpArrSize],
                    (sLBA + nLBA + 1), (nLBA * lbaFormat.MS), lbaFormat.MS);
            }
            writeCmd->SetSLBA(sLBA);
            readCmd->SetSLBA(sLBA);

            enableLog = false;
            if ((sLBA <= maxWrBlks) || (sLBA >= (ncap - 2 * maxWrBlks)))
                enableLog = true;
            work = str(boost::format("metaID.%d.SLBA.%ld") % meta[i] % sLBA);

            LOG_NRM("Sending write and read commands through ioq's");
            IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq,
                iocq, writeCmd, work, enableLog);

            IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq,
                iocq, readCmd, work, enableLog);

            VerifyDataPat(readCmd, writeCmd, metaBuffSz);
        }
    }
}


void
StartingLBAMeta_r10b::VerifyDataPat(SharedReadPtr readCmd,
    SharedWritePtr writeCmd, uint64_t metaBuffSz)
{
    LOG_NRM("Compare read vs written data to verify");
    SharedMemBufferPtr rdPayload = readCmd->GetRWPrpBuffer();
    SharedMemBufferPtr wrPayload = writeCmd->GetRWPrpBuffer();
    if (rdPayload->Compare(wrPayload) == false) {
        readCmd->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "ReadCmd"),
            "Read command");
        rdPayload->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "ReadPayload"),
            "Data read from media miscompared from written");
        wrPayload->Dump(
            FileSystem::PrepDumpFile(mGrpName, mTestName, "WrittenPayload"),
            "Data read from media miscompared from written");
        throw FrmwkEx(HERE, "Data miscompare");
    }

    // If meta data is allocated then compare meta data.
    if (writeCmd->GetMetaBuffer() != NULL) {
        const uint8_t *metaRdBuff = readCmd->GetMetaBuffer();
        const uint8_t *metaWrBuff = writeCmd->GetMetaBuffer();
        if (memcmp(metaRdBuff, metaWrBuff, metaBuffSz)) {
            readCmd->Dump(
                FileSystem::PrepDumpFile(mGrpName, mTestName, "ReadCmdMeta"),
                "Read command with meta data");
            writeCmd->Dump(
                FileSystem::PrepDumpFile(mGrpName, mTestName, "WriteCmdMeta"),
                "Write command with meta data");
            throw FrmwkEx(HERE, "Meta data miscompare, Meta Sz %d", metaBuffSz);
        }
    }
}


void
StartingLBAMeta_r10b::CreateIOQs(SharedASQPtr asq, SharedACQPtr acq,
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


void
StartingLBAMeta_r10b::ResizeDataBuf(SharedReadPtr &readCmd,
    SharedWritePtr &writeCmd, ConstSharedIdentifyPtr namSpcPtr,
    uint64_t maxWrBlks, send_64b_bitmask prpBitmask)
{
    LBAFormat lbaFormat = namSpcPtr->GetLBAFormat();
    uint64_t lbaDataSize = (1 << lbaFormat.LBADS);

    SharedMemBufferPtr readMem = readCmd->GetRWPrpBuffer();
    SharedMemBufferPtr writeMem = writeCmd->GetRWPrpBuffer();

    switch (gInformative->IdentifyNamespace(namSpcPtr)) {
    case Informative::NS_BARE:
        throw FrmwkEx(HERE, "Namspc type cannot be BARE.");
    case Informative::NS_METAS:
        LOG_NRM("Resized max rd/wr blks to %ld for separate meta", maxWrBlks);
        writeMem->Init(maxWrBlks * lbaDataSize);
        readMem->Init(maxWrBlks * lbaDataSize);
        break;
    case Informative::NS_METAI:
        LOG_NRM("Resized max rd/wr blks to %ld for integrated meta", maxWrBlks);
        writeMem->Init(maxWrBlks * (lbaDataSize + lbaFormat.MS));
        readMem->Init(maxWrBlks * (lbaDataSize + lbaFormat.MS));
        break;
    case Informative::NS_E2ES:
    case Informative::NS_E2EI:
    throw FrmwkEx(HERE, "Deferring work to handle this case in future");
    break;
    }

    writeCmd->SetPrpBuffer(prpBitmask, writeMem);
    writeCmd->SetNLB(maxWrBlks - 1); // 0 based value.

    readCmd->SetPrpBuffer(prpBitmask, readMem);
    readCmd->SetNLB(maxWrBlks - 1); // 0 based value.
}


}   // namespace
