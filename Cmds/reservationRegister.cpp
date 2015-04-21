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

#include "reservationRegister.h"


SharedReservationRegisterPtr ReservationRegister::NullReservationRegisterPtr;
const uint8_t ReservationRegister::Opcode = 0x0D;


ReservationRegister::ReservationRegister() : Cmd(Trackable::OBJ_RESERVATIONREGISTER)
{
    Init(Opcode, DATADIR_TO_DEVICE, 64);

    // No cmd should ever be created which violates these masking possibilities
    send_64b_bitmask allowPrpMask = (send_64b_bitmask) (MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    SetPrpAllowed(allowPrpMask);
}


ReservationRegister::~ReservationRegister()
{
}


void
ReservationRegister::SetCPTPL(uint8_t val)
{
    LOG_NRM("Setting CPTPL = 0x%02X", val);
    SetBit(val & 0x1, 10, 30);
    SetBit(val & 0x2, 10, 31);
}

uint8_t
ReservationRegister::GetCPTPL() const
{
    LOG_NRM("Getting CPTPL");
    return GetBit(10, 30) | (GetBit(10, 31) << 1);
}


void
ReservationRegister::SetIEKEY(bool val)
{
    LOG_NRM("Setting IEKEY = %d", val ? 1 : 0);
    SetBit(val, 10, 3);
}


bool
ReservationRegister::GetIEKEY() const
{
    LOG_NRM("Getting IEKEY");
    return GetBit(10, 3);
}


void
ReservationRegister::SetRREGA(uint8_t val)
{
    LOG_NRM("Setting RREGA = %d", val);
    SetBit(val & 0x1, 10, 0);
    SetBit(val & 0x2, 10, 1);
    SetBit(val & 0x4, 10, 2);
}


uint8_t
ReservationRegister::GetRREGA() const
{
    LOG_NRM("Getting RREGA");
    return GetBit(10, 0) | (GetBit(10, 1) << 1) | (GetBit(10, 2) << 2);
}
