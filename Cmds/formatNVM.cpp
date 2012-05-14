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

#include "formatNVM.h"

#define BYTE_BITMASK_SES        0x0e
#define BYTE_BITMASK_PI         0xe0
#define BYTE_BITMASK_LBAF       0x0f

SharedFormatNVMPtr FormatNVM::NullFormatNVMPtr;
const uint8_t FormatNVM::Opcode = 0x80;


FormatNVM::FormatNVM() : Cmd(Trackable::OBJ_FORMATNVM)
{
    Cmd::Init(Opcode, DATADIR_NONE, 64);
}


FormatNVM::~FormatNVM()
{
}


void
FormatNVM::SetSES(uint8_t ses)
{
    uint8_t work;

    const uint8_t MAX_VALUE = 0x07;
    if (ses > MAX_VALUE)
        throw FrmwkEx(HERE, "Illegal ses %d > %d(max)", MAX_VALUE);

    LOG_NRM("Setting SES = 0x%02X", ses);
    work = GetByte(10, 1);
    work &= ~BYTE_BITMASK_SES;
    work |= ((ses << 1) & BYTE_BITMASK_SES);
    SetByte(work, 10, 1);
}


uint8_t
FormatNVM::GetSES() const
{
    uint8_t work;
    LOG_NRM("Getting SES");
    work = GetByte(10, 1);
    return ((work & BYTE_BITMASK_SES) >> 1);
}


void
FormatNVM::SetPIL(bool pil)
{
    LOG_NRM("Setting PILL = %d", pil);
    SetBit(pil, 10, 8);
}


bool
FormatNVM::GetPIL() const
{
    LOG_NRM("Getting PIL");
    return GetBit(10, 8);
}


void
FormatNVM::SetPI(uint8_t pi)
{
    uint8_t work;

    const uint8_t MAX_VALUE = 0x07;
    if (pi > MAX_VALUE)
        throw FrmwkEx(HERE, "Illegal pi %d > %d(max)", MAX_VALUE);

    LOG_NRM("Setting PI = 0x%02X", pi);
    work = GetByte(10, 0);
    work &= ~BYTE_BITMASK_PI;
    work |= ((pi << 5) & BYTE_BITMASK_PI);
    SetByte(work, 10, 0);
}


uint8_t
FormatNVM::GetPI() const
{
    uint8_t work;
    LOG_NRM("Getting PI");
    work = GetByte(10, 0);
    return ((work & BYTE_BITMASK_PI) >> 5);
}


void
FormatNVM::SetMS(bool ms)
{
    LOG_NRM("Setting MS = %d", ms);
    SetBit(ms, 10, 4);
}


bool
FormatNVM::GetMS() const
{
    LOG_NRM("Getting MS");
    return GetBit(10, 4);
}


void
FormatNVM::SetLBAF(uint8_t lbaf)
{
    uint8_t work;

    const uint8_t MAX_VALUE = 0x0F;
    if (lbaf > MAX_VALUE)
        throw FrmwkEx(HERE, "Illegal lbaf %d > %d(max)", MAX_VALUE);

    LOG_NRM("Setting LBAF = 0x%02X", lbaf);
    work = GetByte(10, 0);
    work &= ~BYTE_BITMASK_LBAF;
    work |= ((lbaf << 0) & BYTE_BITMASK_LBAF);
    SetByte(work, 10, 0);
}


uint8_t
FormatNVM::GetLBAF() const
{
    uint8_t work;
    LOG_NRM("Getting LBAF");
    work = GetByte(10, 0);
    return ((work & BYTE_BITMASK_LBAF) >> 0);
}


