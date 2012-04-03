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
#include "prpOffsetSinglePgSingleBlk_r10b.h"
#include "grpDefs.h"


namespace GrpNVMWriteReadCombo {


PRPOffsetSinglePgSingleBlk_r10b::PRPOffsetSinglePgSingleBlk_r10b(int fd,
    string mGrpName, string mTestName, ErrorRegs errRegs) :
    Test(fd, mGrpName, mTestName, SPECREV_10b, errRegs)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Vary buffer offset for single-page/single-blk "
                            "against PRP1/PRP2.");
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
        "4B; alternate the data pattern between byteK, word++ for each write. "
        "NOTE: Since PRP1 is the only effective ptr, then PRP2 becomes "
        "reserved. Reserved fields must not be checked by the recipient. "
        "Verify this fact by injecting random 64b values into PRP2 by "
        "alternating random number, then zero, for each and every write/read "
        "cmd issued to the DUT. Always initiate the random gen with seed 17 "
        "before this test starts.");
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


void
PRPOffsetSinglePgSingleBlk_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */
    string work;
    unsigned int seed = 17;

    /* Initialize random seed to 17 */
    srand (seed);

    // Lookup objs which were created in a prior test within group
    SharedIOSQPtr iosq = CAST_TO_IOSQ(gRsrcMngr->GetObj(IOSQ_GROUP_ID));
    SharedIOCQPtr iocq = CAST_TO_IOCQ(gRsrcMngr->GetObj(IOCQ_GROUP_ID));

    Informative::Namspc namspcData = gInformative->Get1stBareMetaE2E();
    send_64b_bitmask prpBitmask = (send_64b_bitmask)(MASK_PRP1_PAGE);
    if (namspcData.type != Informative::NS_BARE) {
        prpBitmask = (send_64b_bitmask)(prpBitmask | MASK_MPTR);
        LBAFormat lbaFormat = namspcData.idCmdNamspc->GetLBAFormat();
        if (gRsrcMngr->SetMetaAllocSize(lbaFormat.MS) == false)
            throw FrmwkEx();
    }
    uint64_t lbaDataSize = namspcData.idCmdNamspc->GetLBADataSize();

    uint8_t mpsRegVal;
    if (gCtrlrConfig->GetMPS(mpsRegVal) == false)
        throw FrmwkEx("Unable to get MPS value from CC.");
    uint64_t X =  (uint64_t)(1 << (mpsRegVal + 12)) - lbaDataSize;

    SharedWritePtr writeCmd = CreateWriteCmd(namspcData);
    SharedReadPtr readCmd = CreateReadCmd(namspcData);

    MemBuffer::DataPattern dataPattern;
    MetaData::DataPattern metaDataPattern;
    uint64_t wrVal;
    uint32_t prp2RandVal[2];
    for (uint64_t memOffset = 0; memOffset <= X; memOffset += 4) {
        if ((memOffset % 8) != 0) {
            dataPattern = MemBuffer::DATAPAT_CONST_8BIT;
            metaDataPattern = MetaData::DATAPAT_CONST_8BIT;
            wrVal = memOffset;
            prp2RandVal[0] = rand_r(&seed);
            prp2RandVal[1] = rand_r(&seed);
            work = str(boost::format("dataPat.constb.memOff.%d") % memOffset);
        } else {
            dataPattern = MemBuffer::DATAPAT_INC_16BIT;
            metaDataPattern = MetaData::DATAPAT_INC_16BIT;
            wrVal = memOffset;
            prp2RandVal[0] = 0;
            prp2RandVal[1] = 0;
            work = str(boost::format("dataPat.incw.memOff.%d") % memOffset);
        }
        SharedMemBufferPtr writeMem = SharedMemBufferPtr(new MemBuffer());
        writeMem->InitOffset1stPage(lbaDataSize, memOffset, false);
        writeCmd->SetPrpBuffer(prpBitmask, writeMem);
        writeMem->SetDataPattern(dataPattern, wrVal);
        if (namspcData.type != Informative::NS_BARE)
            writeCmd->SetMetaDataPattern(metaDataPattern, wrVal);

        // Set 64 bits of PRP2 in CDW 8 & 9 with random or zero for write cmd.
        writeCmd->SetDword(prp2RandVal[0], 8);
        writeCmd->SetDword(prp2RandVal[1], 9);

        IO::SendCmdToHdw(mGrpName, mTestName, DEFAULT_CMD_WAIT_ms, iosq, iocq,
            writeCmd, work, true);

        SharedMemBufferPtr readMem = SharedMemBufferPtr(new MemBuffer());
        readMem->InitOffset1stPage(lbaDataSize, memOffset, false);
        readCmd->SetPrpBuffer(prpBitmask, readMem);

        // Set 64 bits of PRP2 in CDW 8 & 9 with random or zero for read cmd.
        readCmd->SetDword(prp2RandVal[0], 8);
        readCmd->SetDword(prp2RandVal[1], 9);

        IO::SendCmdToHdw(mGrpName, mTestName, DEFAULT_CMD_WAIT_ms, iosq, iocq,
            readCmd, work, true);

        VerifyDataPattern(readCmd, dataPattern, wrVal);
    }
}


SharedWritePtr
PRPOffsetSinglePgSingleBlk_r10b::CreateWriteCmd(Informative::Namspc namspcData)
{
    SharedWritePtr writeCmd = SharedWritePtr(new Write());

    if (namspcData.type == Informative::NS_META) {
        writeCmd->AllocMetaBuffer();
    } else if (namspcData.type == Informative::NS_E2E) {
        writeCmd->AllocMetaBuffer();
        LOG_ERR("Deferring E2E namspc work to the future");
        throw FrmwkEx("Need to add CRC's to correlate to buf pattern");
    }

    writeCmd->SetNSID(namspcData.id);
    writeCmd->SetNLB(0);
    return writeCmd;
}


SharedReadPtr
PRPOffsetSinglePgSingleBlk_r10b::CreateReadCmd(Informative::Namspc namspcData)
{
    SharedReadPtr readCmd = SharedReadPtr(new Read());

    if (namspcData.type == Informative::NS_META) {
        readCmd->AllocMetaBuffer();
    } else if (namspcData.type == Informative::NS_E2E) {
        readCmd->AllocMetaBuffer();
        LOG_ERR("Deferring E2E namspc work to the future");
        throw FrmwkEx("Need to add CRC's to correlate to buf pattern");
    }

    readCmd->SetNSID(namspcData.id);
    readCmd->SetNLB(0);
    return readCmd;
}


void
PRPOffsetSinglePgSingleBlk_r10b::VerifyDataPattern(SharedReadPtr readCmd,
    MemBuffer::DataPattern dataPattern, uint64_t wrVal)
{
    LOG_NRM("Compare read vs written data to verify");
    uint16_t mWrVal = (uint16_t)wrVal;
    if (dataPattern == MemBuffer::DATAPAT_INC_16BIT) {
        const uint16_t *rdBuffPtr = (const uint16_t *)readCmd->GetROPrpBuffer();
        for (uint64_t i = 0; i <
            (readCmd->GetPrpBufferSize() / sizeof(uint16_t)); i++) {
            if (*rdBuffPtr++ != mWrVal++) {
                readCmd->Dump(
                    FileSystem::PrepLogFile(mGrpName, mTestName, "ReadPayload"),
                    "Data read from media miscompared from written");
                throw FrmwkEx("Read data mismatch for 16bit inc prp data "
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
                        FileSystem::PrepLogFile(mGrpName, mTestName,
                        "MetaPayload"),
                        "Meta Data read from media miscompared from written");
                    throw FrmwkEx("Read data mismatch for 16bit inc meta data "
                        "read ptr: 0x%08X, read val: 0x%02X, write val: 0x%02X",
                        rdBuffPtr, *rdBuffPtr, mWrVal);
                }
            }
        }
    } else if (dataPattern == MemBuffer::DATAPAT_CONST_8BIT) {
        const uint8_t *rdBuffPtr = readCmd->GetROPrpBuffer();
        for (uint64_t i = 0; i <
            (readCmd->GetPrpBufferSize() / sizeof(uint8_t)); i++) {
            if (*rdBuffPtr++ != (uint8_t)wrVal) {
                readCmd->Dump(
                    FileSystem::PrepLogFile(mGrpName, mTestName, "ReadPayload"),
                    "Data read from media miscompared from written");
                throw FrmwkEx("Read data mismatch for 8bit const data "
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
                        FileSystem::PrepLogFile(mGrpName, mTestName,
                        "MetaPayload"),
                        "Meta Data read from media miscompared from written");
                    throw FrmwkEx("Read data mismatch for 8bit const meta data "
                        "read ptr: 0x%08X, read val: 0x%02X, write val: 0x%02X",
                        rdBuffPtr, *rdBuffPtr, wrVal);
                }
            }
        }
    }
}

}   // namespace
