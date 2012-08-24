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
#include "nlbaMeta_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/io.h"
#include "../Utils/irq.h"


namespace GrpNVMWriteReadCombo {

// Maximum number of bits for logical blks (NLB) in cmd DWORD 12 for rd/wr cmd.
#define CDW12_NLB_BITS          16


NLBAMeta_r10b::NLBAMeta_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Verify all values of Number of Log Blk (NLB) for meta namspcs");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "For all meta namspcs from Identify.NN; For each namspc issue "
        "consecutive write cmds and approp metadata, each staring at "
        "LBA 0 by looping thru all values for DW12.NLB from 0 to {0xffff | "
        "(Identify.MDTS / Identify.LBAF[Identify.FLBAS].LBADS) | NCAP} "
        "which ever is less. Each write cmd should use a new data pattern "
        "by rolling through {byte++, byteK, word++, wordK, dword++, dwordK}. "
        "After each write cmd completes issue a correlating read cmd "
        "through the same parameters verifying the data pattern, and the "
        "metadata which also has the same pattern.");
}


NLBAMeta_r10b::~NLBAMeta_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


NLBAMeta_r10b::
NLBAMeta_r10b(const NLBAMeta_r10b &other) :
    Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


NLBAMeta_r10b &
NLBAMeta_r10b::operator=(const NLBAMeta_r10b
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
NLBAMeta_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
NLBAMeta_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * None.
     * \endverbatim
     */
    string work;
    bool enableLog;
    SharedIOSQPtr iosq;
    SharedIOCQPtr iocq;
    ConstSharedIdentifyPtr namSpcPtr;
    uint64_t metaBuffSz;

    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Create admin queues ACQ and ASQ");
    SharedACQPtr acq = SharedACQPtr(new ACQ(gDutFd));
    acq->Init(5);

    SharedASQPtr asq = SharedASQPtr(new ASQ(gDutFd));
    asq->Init(5);

    ConstSharedIdentifyPtr idCmdCtrlr = gInformative->GetIdentifyCmdCtrlr();
    uint32_t maxDtXferSz = idCmdCtrlr->GetMaxDataXferSize();

    LOG_NRM("Prepare read and write cmds to utilize");
    SharedWritePtr writeCmd = SharedWritePtr(new Write());
    SharedMemBufferPtr writeMem = SharedMemBufferPtr(new MemBuffer());

    SharedReadPtr readCmd = SharedReadPtr(new Read());
    SharedMemBufferPtr readMem = SharedMemBufferPtr(new MemBuffer());

    send_64b_bitmask prpBitmask = (send_64b_bitmask)
        (MASK_PRP1_PAGE | MASK_PRP2_PAGE | MASK_PRP2_LIST);

    DataPattern dataPat[] = {
        DATAPAT_INC_8BIT,
        DATAPAT_CONST_8BIT,
        DATAPAT_INC_16BIT,
        DATAPAT_CONST_16BIT,
        DATAPAT_INC_32BIT,
        DATAPAT_CONST_32BIT
    };
    uint64_t dpArrSize = sizeof(dataPat) / sizeof(dataPat[0]);

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

        namSpcPtr = gInformative->GetIdentifyCmdNamspc(meta[i]);
        LBAFormat lbaFormat = namSpcPtr->GetLBAFormat();
        uint64_t lbaDataSize = (1 << lbaFormat.LBADS);
        uint64_t maxWrBlks = (1 << CDW12_NLB_BITS);     // 1- based value.
        uint64_t ncap = namSpcPtr->GetValue(IDNAMESPC_NCAP);
        maxWrBlks = (maxWrBlks < ncap) ? maxWrBlks : ncap; // limit by ncap.

        Informative::NamspcType nsType =
            gInformative->IdentifyNamespace(namSpcPtr);
        switch (nsType) {
        case Informative::NS_BARE:
            throw FrmwkEx(HERE, "Namspc type cannot be BARE.");
        case Informative::NS_METAS:
            LOG_NRM("Process for separate meta buffer");
            if (maxDtXferSz != 0)
                maxWrBlks = MIN(maxWrBlks, (maxDtXferSz / lbaDataSize));
            metaBuffSz = maxWrBlks * lbaFormat.MS;
            if (gRsrcMngr->SetMetaAllocSize(metaBuffSz) == false)
                throw FrmwkEx(HERE);
            writeCmd->AllocMetaBuffer();
            readCmd->AllocMetaBuffer();
            break;
        case Informative::NS_METAI:
            LOG_NRM("Process for integrated meta buffer");
            if (maxDtXferSz != 0) {
                maxWrBlks = MIN(maxWrBlks,
                    (maxDtXferSz / (lbaDataSize + lbaFormat.MS)));
            }
            break;
        case Informative::NS_E2ES:
        case Informative::NS_E2EI:
            throw FrmwkEx(HERE, "Deferring work to handle this case in future");
            break;
        }
        writeCmd->SetNSID(meta[i]);
        readCmd->SetNSID(meta[i]);

        // If we execute for every possible LBA, then it will take hrs to
        // complete. So incrementing LBA in powers of 2 is a best effort
        // solution to minimize the execution time.
        // lbaPow2 = {2, 4, 8, 16, 32, 64, ..., 0x10000}
        for (uint64_t lbaPow2 = 2; lbaPow2 <= maxWrBlks; lbaPow2 <<= 1) {
            // nLBA = {(1, 2, 3), (3, 4, 5), ..., (0xFFFF, 0x10000, 0x10001)}
            for (uint64_t nLBA = (lbaPow2 - 1); nLBA <= (lbaPow2 + 1); nLBA++) {
                LOG_NRM("Processing LBA #%ld of %ld", nLBA, maxWrBlks);
                if (nLBA > maxWrBlks)
                    break;

                LOG_NRM("Set rd/wr mem based on the namspc type %d", nsType);
                metaBuffSz = 0;
                switch (nsType) {
                case Informative::NS_BARE:
                    throw FrmwkEx(HERE, "Namspc type cannot be BARE.");
                case Informative::NS_METAS:
                    writeMem->Init(nLBA * lbaDataSize);
                    readMem->Init(nLBA * lbaDataSize);
                    metaBuffSz = nLBA * lbaFormat.MS;
                    writeCmd->SetMetaDataPattern
                        (dataPat[(nLBA - 1) % dpArrSize], nLBA);
                    break;
                case Informative::NS_METAI:
                    writeMem->Init(nLBA * (lbaDataSize + lbaFormat.MS));
                    readMem->Init(nLBA * (lbaDataSize + lbaFormat.MS));
                    break;
                case Informative::NS_E2ES:
                case Informative::NS_E2EI:
                    throw FrmwkEx(HERE, "Deferring work to handle in future");
                    break;
                }

                writeCmd->SetPrpBuffer(prpBitmask, writeMem);
                writeCmd->SetNLB(nLBA - 1); // 0 based value.
                writeMem->SetDataPattern(dataPat[(nLBA - 1) % dpArrSize], nLBA);

                readCmd->SetPrpBuffer(prpBitmask, readMem);
                readCmd->SetNLB(nLBA - 1); // 0 based value.

                enableLog = false;
                if ((nLBA <= 8) || (nLBA >= (maxWrBlks - 8)))
                    enableLog = true;
                work = str(boost::format("NSID.%d.LBA.%ld") % meta[i] % nLBA);

                IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
                    iosq, iocq, writeCmd, work, enableLog);

                IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
                    iosq, iocq, readCmd, work, enableLog);

                VerifyDataPat(readCmd, writeCmd, metaBuffSz);
            }
        }
    }
}


void
NLBAMeta_r10b::VerifyDataPat(SharedReadPtr readCmd, SharedWritePtr writeCmd,
    uint64_t metaBuffSz)
{
    LOG_NRM("Compare read vs written data to verify");
    SharedMemBufferPtr wrPayload = writeCmd->GetRWPrpBuffer();
    SharedMemBufferPtr rdPayload = readCmd->GetRWPrpBuffer();
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
        LOG_NRM("Compare meta data as cmd has meta buffer(size = %ld)"
            " association", metaBuffSz);
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
NLBAMeta_r10b::CreateIOQs(SharedASQPtr asq, SharedACQPtr acq, uint32_t ioqId,
    SharedIOSQPtr &iosq, SharedIOCQPtr &iocq)
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
