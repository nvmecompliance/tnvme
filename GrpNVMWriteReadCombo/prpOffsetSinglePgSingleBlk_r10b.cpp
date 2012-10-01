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
#include "prpOffsetSinglePgSingleBlk_r10b.h"
#include "grpDefs.h"


namespace GrpNVMWriteReadCombo {


PRPOffsetSinglePgSingleBlk_r10b::PRPOffsetSinglePgSingleBlk_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Vary buff offset for single-page/single-blk against PRP1/PRP2.");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Search for 1 of the following namspcs to run test. Find 1st bare "
        "namspc, or find 1st meta namspc, or find 1st E2E namspc. Issue "
        "identical write cmd starting at LBA 0, sending a single block with "
        "approp meta/E2E requirements if necessary, where write executes the "
        "ALGO listed below; subsequently after each write a read must verify "
        "the data pattern for success, any metadata will also need to be "
        "verified for it will have the same data pattern as the data. ALGO) "
        "Alloc discontig memory; vary offset into 1st memory page from 0 to X, "
        "where X = (CC.MPS - Identify.LBAF[Identify.FLBAS].LBADS) in steps of "
        "4B; alternate the data pattern between byteK, word++ for each write.");
}


PRPOffsetSinglePgSingleBlk_r10b::~PRPOffsetSinglePgSingleBlk_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


PRPOffsetSinglePgSingleBlk_r10b::
PRPOffsetSinglePgSingleBlk_r10b(const PRPOffsetSinglePgSingleBlk_r10b &other) :
    Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


PRPOffsetSinglePgSingleBlk_r10b &
PRPOffsetSinglePgSingleBlk_r10b::operator=(const PRPOffsetSinglePgSingleBlk_r10b
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
PRPOffsetSinglePgSingleBlk_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
PRPOffsetSinglePgSingleBlk_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */
    string work;
    int64_t X;
    bool enableLog;

    // Lookup objs which were created in a prior test within group
    SharedIOSQPtr iosq = CAST_TO_IOSQ(gRsrcMngr->GetObj(IOSQ_GROUP_ID));
    SharedIOCQPtr iocq = CAST_TO_IOCQ(gRsrcMngr->GetObj(IOCQ_GROUP_ID));

    LOG_NRM("Get namspc and determine LBA size");
    Informative::Namspc namspcData = gInformative->Get1stBareMetaE2E();
    send_64b_bitmask prpBitmask = (send_64b_bitmask)(MASK_PRP1_PAGE);
    LBAFormat lbaFormat = namspcData.idCmdNamspc->GetLBAFormat();
    uint64_t lbaDataSize = namspcData.idCmdNamspc->GetLBADataSize();

    uint8_t mpsRegVal;
    if (gCtrlrConfig->GetMPS(mpsRegVal) == false)
        throw FrmwkEx(HERE, "Unable to get MPS value from CC.");

    switch (namspcData.type) {
    case Informative::NS_BARE:
        X =  (int64_t)(1 << (mpsRegVal + 12)) - lbaDataSize;
        break;
    case Informative::NS_METAS:
        if (gRsrcMngr->SetMetaAllocSize(lbaFormat.MS) == false)
            throw FrmwkEx(HERE);
        X =  (int64_t)(1 << (mpsRegVal + 12)) - lbaDataSize;
        break;
    case Informative::NS_METAI:
        X =  (int64_t)(1 << (mpsRegVal + 12)) - (lbaDataSize + lbaFormat.MS);
        break;
    case Informative::NS_E2ES:
    case Informative::NS_E2EI:
        throw FrmwkEx(HERE, "Deferring work to handle this case in future");
        break;
    }
    if (X <= 0) {
        LOG_WARN("CC.MPS (0x%04X) < lba data size(LBADS) + MS "
            "(0x08%lX + 0x04%X) ; Can't run test.", (1 << (mpsRegVal + 12)),
            lbaDataSize, lbaFormat.MS);
        return;
    }

    LOG_NRM("Prepare cmds to send to the queues.");
    SharedWritePtr writeCmd = CreateWriteCmd(namspcData);
    SharedReadPtr readCmd = CreateReadCmd(namspcData);

    DataPattern dataPattern;
    uint64_t wrVal;
    for (int64_t pgOff = 0; pgOff <= X; pgOff += 4) {
        LOG_NRM("Processing at page offset #%ld", pgOff);
        if ((pgOff % 8) != 0) {
            dataPattern = DATAPAT_CONST_8BIT;
            wrVal = pgOff;
            work = str(boost::format("dataPat.constb.memOff.%d") % pgOff);
        } else {
            dataPattern = DATAPAT_INC_16BIT;
            wrVal = pgOff;
            work = str(boost::format("dataPat.incw.memOff.%d") % pgOff);
        }
        SharedMemBufferPtr writeMem = SharedMemBufferPtr(new MemBuffer());

        switch (namspcData.type) {
        case Informative::NS_BARE:
            writeMem->InitOffset1stPage(lbaDataSize, pgOff, false);
            break;
        case Informative::NS_METAS:
            writeMem->InitOffset1stPage(lbaDataSize, pgOff, false);
            writeCmd->SetMetaDataPattern(dataPattern, wrVal);
            break;
        case Informative::NS_METAI:
            writeMem->InitOffset1stPage
                (lbaDataSize + lbaFormat.MS, pgOff, false);
            break;
        case Informative::NS_E2ES:
        case Informative::NS_E2EI:
            throw FrmwkEx(HERE, "Deferring work to handle this case in future");
            break;
        }
        writeCmd->SetPrpBuffer(prpBitmask, writeMem);
        writeMem->SetDataPattern(dataPattern, wrVal);

        enableLog = false;
        if ((pgOff <= 8) || (pgOff >= (X - 8)))
            enableLog = true;

        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq,
            writeCmd, work, enableLog);

        SharedMemBufferPtr readMem = SharedMemBufferPtr(new MemBuffer());
        switch (namspcData.type) {
        case Informative::NS_BARE:
            readMem->InitOffset1stPage(lbaDataSize, pgOff, false);
            break;
        case Informative::NS_METAS:
            readMem->InitOffset1stPage(lbaDataSize, pgOff, false);
            break;
        case Informative::NS_METAI:
            readMem->InitOffset1stPage
                (lbaDataSize + lbaFormat.MS, pgOff, false);
            break;
        case Informative::NS_E2ES:
        case Informative::NS_E2EI:
            throw FrmwkEx(HERE, "Deferring work to handle this case in future");
            break;
        }
        readCmd->SetPrpBuffer(prpBitmask, readMem);

        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq,
            readCmd, work, enableLog);

        VerifyDataPattern(readCmd, dataPattern, wrVal);
    }
}


SharedWritePtr
PRPOffsetSinglePgSingleBlk_r10b::CreateWriteCmd(Informative::Namspc namspcData)
{
    SharedWritePtr writeCmd = SharedWritePtr(new Write());

    switch (namspcData.type) {
    case Informative::NS_BARE:
        break;
    case Informative::NS_METAS:
        writeCmd->AllocMetaBuffer();
        break;
    case Informative::NS_METAI:
        break;
    case Informative::NS_E2ES:
    case Informative::NS_E2EI:
        throw FrmwkEx(HERE, "Deferring work to handle this case in future");
        break;
    }

    writeCmd->SetNSID(namspcData.id);
    writeCmd->SetNLB(0);
    return writeCmd;
}


SharedReadPtr
PRPOffsetSinglePgSingleBlk_r10b::CreateReadCmd(Informative::Namspc namspcData)
{
    SharedReadPtr readCmd = SharedReadPtr(new Read());

    switch (namspcData.type) {
    case Informative::NS_BARE:
        break;
    case Informative::NS_METAS:
        readCmd->AllocMetaBuffer();
        break;
    case Informative::NS_METAI:
        break;
    case Informative::NS_E2ES:
    case Informative::NS_E2EI:
        throw FrmwkEx(HERE, "Deferring work to handle this case in future");
        break;
    }

    readCmd->SetNSID(namspcData.id);
    readCmd->SetNLB(0);
    return readCmd;
}


void
PRPOffsetSinglePgSingleBlk_r10b::VerifyDataPattern(SharedReadPtr readCmd,
    DataPattern dataPattern, uint64_t wrVal)
{
    LOG_NRM("Compare read vs written data to verify");
    uint16_t mWrVal = (uint16_t)wrVal;
    if (dataPattern == DATAPAT_INC_16BIT) {
        const uint16_t *rdBuffPtr = (const uint16_t *)readCmd->GetROPrpBuffer();
        for (uint64_t i = 0; i <
            (readCmd->GetPrpBufferSize() / sizeof(uint16_t)); i++) {
            if (*rdBuffPtr++ != mWrVal++) {
                readCmd->Dump(
                    FileSystem::PrepDumpFile(mGrpName, mTestName,
                    "ReadPayload"),
                    "Data read from media miscompared from written");
                throw FrmwkEx(HERE, "Read data mismatch for 16bit inc prp data "
                    "read ptr: 0x%08X, read value: 0x%02X, write value: 0x%02X",
                    rdBuffPtr, *rdBuffPtr, mWrVal);
            }
        }
        // Check if meta data exists and then compare meta buffer data.
        const uint16_t *rdMetaPtr = (const uint16_t *)readCmd->GetMetaBuffer();
        mWrVal = (uint16_t)wrVal;
        if (rdMetaPtr) {
            LOG_NRM("Compare read vs written meta data to verify");
            for (uint64_t i = 0; i <
                (readCmd->GetMetaBufferSize() / sizeof(uint16_t)); i++) {
                if (*rdMetaPtr++ != mWrVal++) {
                    readCmd->Dump(
                        FileSystem::PrepDumpFile(mGrpName, mTestName,
                        "MetaPayload"),
                        "Meta Data read from media miscompared from written");
                    throw FrmwkEx(HERE,
                        "Read data mismatch for 16bit inc meta data "
                        "read ptr: 0x%08X, read val: 0x%02X, write val: 0x%02X",
                        rdBuffPtr, *rdBuffPtr, mWrVal);
                }
            }
        }
    } else if (dataPattern == DATAPAT_CONST_8BIT) {
        const uint8_t *rdBuffPtr = readCmd->GetROPrpBuffer();
        for (uint64_t i = 0; i <
            (readCmd->GetPrpBufferSize() / sizeof(uint8_t)); i++) {
            if (*rdBuffPtr++ != (uint8_t)wrVal) {
                readCmd->Dump(
                    FileSystem::PrepDumpFile(mGrpName, mTestName,
                    "ReadPayload"),
                    "Data read from media miscompared from written");
                throw FrmwkEx(HERE, "Read data mismatch for 8bit const data "
                    "read ptr: 0x%08X, read value: 0x%02X, write value: 0x%02X",
                    rdBuffPtr, *rdBuffPtr, wrVal);
            }
        }
        // Check if meta data exists and then compare meta buffer data.
        const uint8_t *rdMetaPtr = readCmd->GetMetaBuffer();
        if (rdMetaPtr) {
            LOG_NRM("Compare read vs written meta data to verify");
            for (uint64_t i = 0; i <
                (readCmd->GetMetaBufferSize() / sizeof(uint8_t)); i++) {
                if (*rdMetaPtr++ != (uint8_t)wrVal) {
                    readCmd->Dump(
                        FileSystem::PrepDumpFile(mGrpName, mTestName,
                        "MetaPayload"),
                        "Meta Data read from media miscompared from written");
                    throw FrmwkEx(HERE,
                        "Read data mismatch for 8bit const meta data "
                        "read ptr: 0x%08X, read val: 0x%02X, write val: 0x%02X",
                        rdBuffPtr, *rdBuffPtr, wrVal);
                }
            }
        }
    }
}

}   // namespace
