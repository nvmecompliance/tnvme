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

#include "read.h"


SharedReadPtr Read::NullReadPtr;
const uint8_t Read::Opcode = 0x02;


Read::Read() : Cmd(Trackable::OBJ_READ)
{
    Init(Opcode, DATADIR_FROM_DEVICE, 64);

    // No cmd should ever be created which violates these masking possibilities
    send_64b_bitmask allowPrpMask = (send_64b_bitmask)
        (MASK_PRP1_PAGE | MASK_PRP2_PAGE | MASK_PRP2_LIST);
    SetPrpAllowed(allowPrpMask);
}


Read::~Read()
{
}


void
Read::SetSLBA(uint64_t lba)
{
    LOG_NRM("Setting SLBA = 0x%016llX", (long long unsigned int)lba);
    SetDword((uint32_t)(lba >> 0), 10);
    SetDword((uint32_t)(lba >> 32), 11);
}


uint64_t
Read::GetSLBA() const
{
    uint64_t lba = 0;
    LOG_NRM("Getting SLBA");
    lba =  (((uint64_t)GetDword(10)) << 0);
    lba |= (((uint64_t)GetDword(11)) << 32);
    return lba;
}


void
Read::SetLR(bool lr)
{
    LOG_NRM("Setting LR = %d", lr ? 1 : 0);
    SetBit(lr, 12, 31);
}


bool
Read::GetLR() const
{
    LOG_NRM("Getting LR");
    return GetBit(12, 31);
}


void
Read::SetFUA(bool fua)
{
    LOG_NRM("Setting FUA = %d", fua ? 1 : 0);
    SetBit(fua, 12, 30);
}


bool
Read::GetFUA() const
{
    LOG_NRM("Getting FUA");
    return GetBit(12, 30);
}


void
Read::SetPRINFO(uint8_t prinfo)
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
Read::GetPRINFO() const
{
    LOG_NRM("Getting PRINFO");
    return (uint8_t)(GetWord(12, 1) >> 10);
}


void
Read::SetNLB(uint16_t nlb)
{
    LOG_NRM("Setting NLB = 0x%04X", nlb);
    SetWord(nlb, 12, 0);
}


uint16_t
Read::GetNLB() const
{
    LOG_NRM("Getting NLB");
    return GetWord(12, 0);
}


void
Read::SetDSMIncompress(bool incompress)
{
    LOG_NRM("Setting DSM-Incmopressible = %d", incompress ? 1 : 0);
    SetBit(incompress, 13, 7);
}


bool
Read::GetDSMIncompress() const
{
    LOG_NRM("Getting DSM-Incmopressible");
    return GetBit(13, 7);
}


void
Read::SetDSMSeqRequest(bool seqReq)
{
    LOG_NRM("Setting DSM-Sequential Request = %d", seqReq? 1 : 0);
    SetBit(seqReq, 13, 6);
}


bool
Read::GetDSMSeqRequest() const
{
    LOG_NRM("Getting DSM-Sequential Request");
    return GetBit(13, 6);
}


void
Read::SetDSMAccessLatent(uint8_t accessLat)
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
Read::GetDSMAccessLatent() const
{
    LOG_NRM("Setting DSM-Access Latency");
    return ((GetByte(13, 0) >> 4) & 0x03);
}



void
Read::SetDSMAccessFreq(uint8_t accessFreq)
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
Read::GetDSMAccessFreq() const
{
    LOG_NRM("Getting DSM-Access Freq");
    return ((GetByte(13, 0) >> 0) & 0x0f);
}



void
Read::SetEILBRT(uint32_t eilbrt)
{
    LOG_NRM("Setting EILBRT = 0x%08X", eilbrt);
    SetDword(eilbrt, 14);
}


uint32_t
Read::GetEILBRT() const
{
    LOG_NRM("Getting EILBRT");
    return GetDword(13);
}


void
Read::SetELBATM(uint16_t elbatm)
{
    LOG_NRM("Setting ELBATM = 0x%04X", elbatm);
    SetWord(elbatm, 15, 1);
}


uint16_t
Read::GetELBATM() const
{
    LOG_NRM("Getting ELBATM");
    return GetWord(15, 1); 
}


void
Read::SetELBAT(uint16_t elbat)
{
    LOG_NRM("Setting ELBAT = 0x%04X", elbat);
    SetWord(elbat, 15, 0);
}


uint16_t
Read::GetELBAT() const
{
    LOG_NRM("Getting ELBAT");
    return GetWord(15, 0);
}

