/*
 * Copyright (c) 2012, Intel Corporation.
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

#include "baseSecurity.h"
#include "globals.h"
#include "../Utils/buffers.h"

#define MASK_SPSP       0x00ffff00


BaseSecurity::BaseSecurity() : Cmd(Trackable::OBJTYPE_FENCE)
{
    // This constructor will throw
}


BaseSecurity::BaseSecurity(Trackable::ObjType objBeingCreated) :
    Cmd(objBeingCreated)
{
    // No cmd should ever be created which violates these masking possibilities
    send_64b_bitmask allowPrpMask = (send_64b_bitmask)
        (MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    SetPrpAllowed(allowPrpMask);
}


BaseSecurity::~BaseSecurity()
{
}


void
BaseSecurity::SetSECP(uint8_t secp)
{
    LOG_NRM("Setting SEPC = 0x%02X", secp);
    SetByte(secp, 10, 3);
}


uint8_t
BaseSecurity::GetSECP() const
{
    LOG_NRM("Getting SECP");
    return GetByte(10, 3);
}


void
BaseSecurity::SetSPSP(uint16_t spsp)
{
    uint32_t work;

    LOG_NRM("Setting SES = 0x%04X", spsp);
    work = GetDword(10);
    work &= ~MASK_SPSP;
    work |= (spsp << 8);
    SetDword(work, 10);
}


uint16_t
BaseSecurity::GetSPSP() const
{
    uint32_t work;

    LOG_NRM("Getting SPSP");
    work = GetDword(10);
    return ((work & MASK_SPSP) >> 8);
}


void
BaseSecurity::SetTL(uint16_t tl)
{
    LOG_NRM("Setting TL = 0x%04X", tl);
    SetWord(tl, 11, 1);
}


uint16_t
BaseSecurity::GetTL() const
{
    LOG_NRM("Getting TL");
    return GetWord(11, 1);
}

