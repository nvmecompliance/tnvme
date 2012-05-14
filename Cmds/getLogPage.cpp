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

#include "getLogPage.h"
#include "globals.h"

#define NUMD_BITMASK        0x0fff

SharedGetLogPagePtr GetLogPage::NullGetLogPagePtr;
const uint8_t GetLogPage::Opcode = 0x02;
const uint16_t GetLogPage::ERRINFO_DATA_SIZE = 64;
const uint16_t GetLogPage::SMART_DATA_SIZE = 512;
const uint16_t GetLogPage::FIRMSLOT_DATA_SIZE = 512;


// Register metrics (get log page error log) to aid interfacing with the dnvme
#define ZZ(a, b, c, d)         { b, c, d },
GetLogPageDataType GetLogPage::mErrLogMetrics[] =
{
    ERRLOG_TABLE
};
#undef ZZ

// Register metrics (get log page smart log) to aid interfacing with the dnvme
#define ZZ(a, b, c, d)         { b, c, d },
GetLogPageDataType GetLogPage::mSmartLogMetrics[] =
{
    SMRTLOG_TABLE
};
#undef ZZ

// Register metrics (get log page FW slot log) to aid interfacing with the dnvme
#define ZZ(a, b, c, d)         { b, c, d },
GetLogPageDataType GetLogPage::mFwLogMetrics[] =
{
    FWLOG_TABLE
};
#undef ZZ


GetLogPage::GetLogPage() : Cmd(Trackable::OBJ_GETLOGPAGE)
{
    Init(Opcode, DATADIR_FROM_DEVICE, 64);


    // No cmd should ever be created which violates these masking possibilities
    send_64b_bitmask allowPrpMask = (send_64b_bitmask)
        (MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    SetPrpAllowed(allowPrpMask);
}


GetLogPage::~GetLogPage()
{
}


void
GetLogPage::SetNUMD(uint16_t numDW)
{
    uint16_t curVal = GetWord(10, 1);
    curVal &= ~NUMD_BITMASK;
    curVal |= (numDW & NUMD_BITMASK);
    SetWord(curVal, 10, 1);
}


uint16_t
GetLogPage::GetNUMD() const
{
    uint16_t curVal = GetWord(10, 1);
    curVal &= NUMD_BITMASK;
    LOG_NRM("Getting NUMD 0x%04X", curVal);
    return curVal;
}


void
GetLogPage::SetLID(uint16_t logID)
{
    LOG_NRM("Setting LID 0x%04X", logID);
    SetWord(logID, 10, 0);
}


uint16_t
GetLogPage::GetLID() const
{
    uint16_t curVal = GetWord(10, 0);
    LOG_NRM("Getting LID 0x%04X", curVal);
    return curVal;
}


void
GetLogPage::Dump(DumpFilename filename, string fileHdr) const
{
    FILE *fp;

    Cmd::Dump(filename, fileHdr);

    // Reopen the file and append the same data in a different format
    if ((fp = fopen(filename.c_str(), "a")) == NULL)
        throw FrmwkEx(HERE, "Failed to open file: %s", filename.c_str());

    fprintf(fp, "\n------------------------------------------------------\n");
    fprintf(fp, "----Detailed decoding of the cmd payload as follows---\n");
    fprintf(fp, "------------------------------------------------------");

    // How do we interpret the data contained herein?
    switch (GetLID()) {
    case LOGID_ERROR_INFO:
        for (int i = 0; i < ERRLOG_FENCE; i++)
            Dump(fp, i, mErrLogMetrics);
        break;

    case LOGID_SMART_HEALTH:
        for (int i = 0; i < SMRTLOG_FENCE; i++)
            Dump(fp, i, mSmartLogMetrics);
        break;

    case LOGID_FW_SLOT:
        for (int i = 0; i < FWLOG_FENCE; i++)
            Dump(fp, i, mFwLogMetrics);
        break;

    default:
        fprintf(fp, "Unable to decode LID field: 0x%04X\n", GetLID());
        break;
    }

    fclose(fp);
}


void
GetLogPage::Dump(FILE *fp, int field, GetLogPageDataType *logData) const
{
    const uint8_t *data;
    const int BUF_SIZE = 20;
    char work[BUF_SIZE];
    string output;
    unsigned long dumpLen = logData[field].length;

    fprintf(fp, "\n%s\n", logData[field].desc);

    data = &((GetROPrpBuffer())[logData[field].offset]);
    if ((logData[field].length + logData[field].offset) > GetPrpBufferSize()) {
        LOG_ERR("Detected illegal definition in XXXLOG_TABLE");
        throw FrmwkEx(HERE, "Reference calc (%d): %d + %d >= %ld", field,
            logData[field].length, logData[field].offset, GetPrpBufferSize());
    }

    unsigned long addr = logData[field].offset;
    for (unsigned long j = 0; j < dumpLen; j++, addr++) {
        if ((j % 16) == 15) {
            snprintf(work, BUF_SIZE, " %02X\n", *data++);
            output += work;
            fprintf(fp, "%s", output.c_str());
            output.clear();
        } else if ((j % 16) == 0) {
            snprintf(work, BUF_SIZE, "0x%08X: %02X",
                (uint32_t)addr, *data++);
            output += work;
        } else {
            snprintf(work, BUF_SIZE, " %02X", *data++);
            output += work;
        }
    }
    if (output.length() != 0)
        fprintf(fp, "%s\n", output.c_str());
}
