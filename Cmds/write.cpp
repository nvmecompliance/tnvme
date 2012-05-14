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

#include "write.h"


SharedWritePtr Write::NullWritePtr;
const uint8_t Write::Opcode = 0x01;


Write::Write() : Cmd(Trackable::OBJ_WRITE)
{
    Init(Opcode, DATADIR_TO_DEVICE, 64);

    // No cmd should ever be created which violates these masking possibilities
    send_64b_bitmask allowPrpMask = (send_64b_bitmask)
        (MASK_PRP1_PAGE | MASK_PRP2_PAGE | MASK_PRP2_LIST);
    SetPrpAllowed(allowPrpMask);
}


Write::~Write()
{
}


void
Write::SetSLBA(uint64_t lba)
{
    LOG_NRM("Setting SLBA = 0x%016llX", (long long unsigned int)lba);
    SetDword((uint32_t)(lba >> 0), 10);
    SetDword((uint32_t)(lba >> 32), 11);
}


uint64_t
Write::GetSLBA() const
{
    uint64_t lba = 0;
    LOG_NRM("Getting SLBA");
    lba =  (((uint64_t)GetDword(10)) << 0);
    lba |= (((uint64_t)GetDword(11)) << 32);
    return lba;
}


void
Write::SetLR(bool lr)
{
    LOG_NRM("Setting LR = %d", lr ? 1 : 0);
    SetBit(lr, 12, 31);
}


bool
Write::GetLR() const
{
    LOG_NRM("Getting LR");
    return GetBit(12, 31);
}


void
Write::SetFUA(bool fua)
{
    LOG_NRM("Setting FUA = %d", fua ? 1 : 0);
    SetBit(fua, 12, 30);
}


bool
Write::GetFUA() const
{
    LOG_NRM("Getting FUA");
    return GetBit(12, 30);
}


void
Write::SetPRINFO(uint8_t prinfo)
{
    LOG_NRM("Setting PRINFO = 0x%01X", prinfo);

    if (prinfo > 0x0f)
        throw FrmwkEx(HERE, "Value to large; must fit within 4 bits");

    uint16_t work = GetWord(12, 1);
    work &= ~0x3C00;
    work |= (prinfo << 10);
    SetWord(work, 12, 1);
}


uint8_t
Write::GetPRINFO() const
{
    LOG_NRM("Getting PRINFO");
    return (uint8_t)(GetWord(12, 1) >> 10);
}


void
Write::SetNLB(uint16_t nlb)
{
    LOG_NRM("Setting NLB = 0x%04X", nlb);
    SetWord(nlb, 12, 0);
}


uint16_t
Write::GetNLB() const
{
    LOG_NRM("Getting NLB");
    return GetWord(12, 0);
}


void
Write::SetDSMIncompress(bool incompress)
{
    LOG_NRM("Setting DSM-Incmopressible = %d", incompress ? 1 : 0);
    SetBit(incompress, 13, 7);
}


bool
Write::GetDSMIncompress() const
{
    LOG_NRM("Getting DSM-Incmopressible");
    return GetBit(13, 7);
}


void
Write::SetDSMSeqRequest(bool seqReq)
{
    LOG_NRM("Setting DSM-Sequential Request = %d", seqReq? 1 : 0);
    SetBit(seqReq, 13, 6);
}


bool
Write::GetDSMSeqRequest() const
{
    LOG_NRM("Getting DSM-Sequential Request");
    return GetBit(13, 6);
}


void
Write::SetDSMAccessLatent(uint8_t accessLat)
{
    LOG_NRM("Setting DSM-Access Latency = 0x%01X", accessLat);

    if (accessLat > 0x03)
        throw FrmwkEx(HERE, "Value to large; must fit within 2 bits");

    uint8_t work = GetByte(13, 0);
    work &= ~0x30;
    work |= (accessLat << 4);
    SetByte(work, 13, 0);
}


uint8_t
Write::GetDSMAccessLatent() const
{
    LOG_NRM("Setting DSM-Access Latency");
    return ((GetByte(13, 0) >> 4) & 0x03);
}



void
Write::SetDSMAccessFreq(uint8_t accessFreq)
{
    LOG_NRM("Setting DSM-Access Freq = 0x%01X", accessFreq);

    if (accessFreq > 0x0f)
        throw FrmwkEx(HERE, "Value to large; must fit within 4 bits");

    uint8_t work = GetByte(13, 0);
    work &= ~0x0f;
    work |= (accessFreq << 0);
    SetByte(work, 13, 0);
}


uint8_t
Write::GetDSMAccessFreq() const
{
    LOG_NRM("Getting DSM-Access Freq");
    return ((GetByte(13, 0) >> 0) & 0x0f);
}



void
Write::SetILBRT(uint32_t ilbrt)
{
    LOG_NRM("Setting ILBRT = 0x%08X", ilbrt);
    SetDword(ilbrt, 14);
}


uint32_t
Write::GetILBRT() const
{
    LOG_NRM("Getting ILBRT");
    return GetDword(13);
}


void
Write::SetLBATM(uint16_t lbatm)
{
    LOG_NRM("Setting LBATM = 0x%04X", lbatm);
    SetWord(lbatm, 15, 1); 
}


uint16_t
Write::GetLBATM() const
{
    LOG_NRM("Getting LBATM");
    return GetWord(15, 1); 
}


void
Write::SetLBAT(uint16_t lbat)
{
    LOG_NRM("Setting LBAT = 0x%04X", lbat);
    SetWord(lbat, 15, 0); 
}


uint16_t
Write::GetLBAT() const
{
    LOG_NRM("Getting LBAT");
    return GetWord(15, 0); 
}

