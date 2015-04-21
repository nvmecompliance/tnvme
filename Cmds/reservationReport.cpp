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

#include "reservationReport.h"

SharedReservationReportPtr ReservationReport::NullReservationReportPtr;
const uint8_t ReservationReport::Opcode = 0x0E;

ReservationReport::ReservationReport() : Cmd(Trackable::OBJ_RESERVATIONREPORT)
{
    Init(Opcode, DATADIR_FROM_DEVICE, 64);

    // No cmd should ever be created which violates these masking possibilities
    send_64b_bitmask allowPrpMask = (send_64b_bitmask) (MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    SetPrpAllowed(allowPrpMask);
}

ReservationReport::~ReservationReport()
{
}

void
ReservationReport::SetNUMD(uint32_t val)
{
    LOG_NRM("Setting NUMD = 0x%08X", val);
    SetDword(val, 10);
}

uint32_t
ReservationReport::GetNUMD() const
{
    LOG_NRM("Getting NUMD");
    return GetDword(10);
}
