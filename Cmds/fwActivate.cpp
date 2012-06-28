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

#include "fwActivate.h"

SharedFWActivatePtr FWActivate::NullFWActivatePtr;
const uint8_t FWActivate::Opcode = 0x10;


FWActivate::FWActivate() : Cmd(Trackable::OBJ_FWACTIVATE)
{
    Init(Opcode, DATADIR_NONE, 64);
}


FWActivate::~FWActivate()
{
}


void
FWActivate::SetAA(uint8_t aa)
{
    LOG_NRM("Setting AA = 0x%01X", aa);

    if (aa > 0x03)
        throw FrmwkEx(HERE, "Value to large; must fit within 2 bits");

    uint8_t work = GetByte(10, 0);
    work &= ~0x18;
    work |= (aa << 3);
    SetByte(work, 10, 0);
}


uint8_t
FWActivate::GetAA() const
{
    LOG_NRM("Getting AA");
    return (uint8_t)((GetByte(10, 0) >> 3) & 0x03);
}


void
FWActivate::SetFS(uint8_t fs)
{
    LOG_NRM("Setting FS = 0x%01X", fs);

    if (fs > 0x07)
        throw FrmwkEx(HERE, "Value to large; must fit within 3 bits");

    uint8_t work = GetByte(10, 0);
    work &= ~0x07;
    work |= fs;
    SetByte(work, 10, 0);
}


uint8_t
FWActivate::GetFS() const
{
    LOG_NRM("Getting FS");
    return (uint8_t)(GetByte(10, 0) & 0x07);
}


