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

#include "cmd.h"
#include "../Utils/buffers.h"

using namespace std;

const uint8_t  Cmd::BITMASK_FUSE_B = 0x03;
const uint32_t Cmd::BITMASK_FUSE_DW = (BITMASK_FUSE_B << 8);


Cmd::Cmd() :
    Trackable(Trackable::OBJTYPE_FENCE),
    mCmdBuf(new MemBuffer())
{
    // This constructor will throw
}


Cmd::Cmd(Trackable::ObjType objBeingCreated) :
    Trackable(objBeingCreated),
    mCmdBuf(new MemBuffer())
{
    mDataDir = DATADIR_NONE;
    mCmdName = GetObjName(objBeingCreated);
}


Cmd::~Cmd()
{
}


void
Cmd::Init(uint8_t opcode, DataDir dataDir, uint16_t cmdSize)
{
    switch (dataDir) {
    case DATADIR_NONE:
    case DATADIR_TO_DEVICE:
    case DATADIR_FROM_DEVICE:
        mDataDir = dataDir;
        break;
    default:
        throw FrmwkEx(HERE, "Illegal data direction specified: %d", dataDir);
    }

    if (cmdSize % sizeof(uint32_t) != 0)
        throw FrmwkEx(HERE, "Illegal cmd size specified: %d", cmdSize);

    // Cmd buffers shall be DWORD aligned according to NVME spec., however
    // user space only has option to spec. QWORD alignment.
    mCmdBuf->InitAlignment(cmdSize, PRP_BUFFER_ALIGNMENT, true, 0);
    SetByte(opcode, 0, 0);
}


uint32_t
Cmd::GetDword(uint8_t whichDW) const
{
    if (whichDW >= GetCmdSizeDW())
        throw FrmwkEx(HERE, "Cmd is not large enough to get requested value");

    return ((uint32_t *)mCmdBuf->GetBuffer())[whichDW];
}


uint16_t
Cmd::GetWord(uint8_t whichDW, uint8_t dwOffset) const
{
    if (whichDW >= GetCmdSizeDW())
        throw FrmwkEx(HERE, "Cmd is not large enough to get requested value");
    else if (dwOffset > 1)
        throw FrmwkEx(HERE, "Illegal DW offset parameter passed: %d", dwOffset);

    return (uint16_t)(GetDword(whichDW) >> (dwOffset * 16));
}


uint8_t
Cmd::GetByte(uint8_t whichDW, uint8_t dwOffset) const
{
    if (whichDW >= GetCmdSizeDW())
        throw FrmwkEx(HERE, "Cmd is not large enough to get requested value");
    else if (dwOffset > 3)
        throw FrmwkEx(HERE, "Illegal DW offset parameter passed: %d", dwOffset);

    return (uint8_t)(GetDword(whichDW) >> (dwOffset * 8));
}


bool
Cmd::GetBit(uint8_t whichDW, uint8_t dwOffset) const
{
    if (whichDW >= GetCmdSizeDW())
        throw FrmwkEx(HERE, "Cmd is not large enough to get requested value");
    else if (dwOffset > 31)
        throw FrmwkEx(HERE, "Illegal DW offset parameter passed: %d", dwOffset);

    return (GetDword(whichDW) & (0x00000001 << dwOffset));
}


void
Cmd::SetDword(uint32_t newVal, uint8_t whichDW)
{
    if (whichDW >= GetCmdSizeDW())
        throw FrmwkEx(HERE, "Cmd is not large enough to set requested value");

    uint32_t *dw = (uint32_t *)mCmdBuf->GetBuffer();
    dw[whichDW] = newVal;
}


void
Cmd::SetWord(uint16_t newVal, uint8_t whichDW, uint8_t dwOffset)
{
    if (whichDW >= GetCmdSizeDW()) {
        throw FrmwkEx(HERE, 
            "Cmd is not large enough (%d DWORDS) to set req'd DWORD %d",
            GetCmdSizeDW(), whichDW);
    } else if (dwOffset > 1) {
        throw FrmwkEx(HERE, "Illegal DW offset parameter passed: %d", dwOffset);
    }
    uint32_t dw = GetDword(whichDW);
    dw &= ~(0x0000ffff << (dwOffset * 16));
    dw |= ((uint32_t)newVal << (dwOffset * 16));
    SetDword(dw, whichDW);
}


void
Cmd::SetByte(uint8_t newVal, uint8_t whichDW, uint8_t dwOffset)
{
    if (whichDW >= GetCmdSizeDW()) {
        throw FrmwkEx(HERE, 
            "Cmd is not large enough (%d DWORDS) to set req'd DWORD %d",
            GetCmdSizeDW(), whichDW);
    } else if (dwOffset > 3) {
        throw FrmwkEx(HERE, "Illegal DW offset parameter passed: %d", dwOffset);
    }
    uint32_t dw = GetDword(whichDW);
    dw &= ~(0x000000ff << (dwOffset * 8));
    dw |= ((uint32_t)newVal << (dwOffset * 8));
    SetDword(dw, whichDW);
}


void
Cmd::SetBit(bool newVal, uint8_t whichDW, uint8_t dwOffset)
{
    if (whichDW >= GetCmdSizeDW()) {
        throw FrmwkEx(HERE,
            "Cmd is not large enough (%d DWORDS) to set req'd DWORD %d",
            GetCmdSizeDW(), whichDW);
    } else if (dwOffset > 31) {
        throw FrmwkEx(HERE, "Illegal DW offset parameter passed: %d", dwOffset);
    }
    uint32_t dw = GetDword(whichDW);
    dw &= ~(0x00000001 << dwOffset);
    if (newVal)
        dw |= (0x00000001 << dwOffset);
    SetDword(dw, whichDW);
}


void
Cmd::SetFUSE(uint8_t newVal)
{
    LOG_NRM("Setting FUSE");
    uint8_t b1 = (GetByte(0, 1) & ~BITMASK_FUSE_B);
    b1 |= (newVal & BITMASK_FUSE_B);
    SetByte(b1, 0, 1);
}


uint8_t
Cmd::GetFUSE() const
{
    LOG_NRM("Getting FUSE");
    return (GetByte(0, 1) & BITMASK_FUSE_B);
}


void
Cmd::SetNSID(uint32_t newVal)
{
    LOG_NRM("Setting NSID = %d (0x%08X)", newVal, newVal);
    SetDword(newVal, 1);
}


uint32_t
Cmd::GetNSID() const
{
    LOG_NRM("Getting NSID");
    return GetDword(1);
}


void
Cmd::LogCmd() const
{
    LOG_NRM("Logging Cmd obj contents....");
    for (int i = 0; i < GetCmdSizeDW(); i++)
        LOG_NRM("Cmd DWORD%d: %s0x%08X", i, (i < 10) ? " " : "", GetDword(i));
}


void
Cmd::SetCID(uint16_t cid)
{
    LOG_NRM("Setting CID=0x%04X", cid);
    SetWord(cid, 0, 1);
}


uint16_t
Cmd::GetCID() const
{
    LOG_NRM("Getting CID");
    return GetWord(0, 1);
}


void
Cmd::Dump(DumpFilename filename, string fileHdr) const
{
    FILE *fp;

    if ((fp = fopen(filename.c_str(), "a")) == NULL)
        throw FrmwkEx(HERE, "Failed to open file: %s", filename.c_str());

    fprintf(fp, "This file: %s\n", filename.c_str());
    fprintf(fp, "%s\n\n", fileHdr.c_str());
    fclose(fp);

    Buffers::Dump(filename, (uint8_t *)mCmdBuf->GetBuffer(), 0, ULONG_MAX,
        mCmdBuf->GetBufSize(), "Cmd contents:");
    PrpData::Dump(filename, "Payload contents:");
    MetaData::Dump(filename, "Meta data contents:");
}
