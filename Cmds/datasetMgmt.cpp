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

#include "datasetMgmt.h"


SharedDatasetMgmtPtr DatasetMgmt::NullDatasetMgmtPtr;
const uint8_t DatasetMgmt::Opcode = 0x09;


DatasetMgmt::DatasetMgmt() : Cmd(Trackable::OBJ_DATASETMGMT)
{
    Init(Opcode, DATADIR_TO_DEVICE, 64);

    // No cmd should ever be created which violates these masking possibilities
    send_64b_bitmask allowPrpMask = (send_64b_bitmask)
        (MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    SetPrpAllowed(allowPrpMask);
}


DatasetMgmt::~DatasetMgmt()
{
}


void
DatasetMgmt::SetNR(uint8_t nr)
{
    LOG_NRM("Setting NR = 0x%02X", nr);
    SetByte(nr, 10, 0);
}


uint8_t
DatasetMgmt::GetNR() const
{
    LOG_NRM("Getting NR");
    return GetByte(10, 0);
}


void
DatasetMgmt::SetAD(bool ad)
{
    LOG_NRM("Setting AD = %d", ad ? 1 : 0);
    SetBit(ad, 11, 2);
}


bool
DatasetMgmt::GetAD() const
{
    LOG_NRM("Getting AD");
    return GetBit(11, 2);
}


void
DatasetMgmt::SetIDW(bool idw)
{
    LOG_NRM("Setting IDW = %d", idw ? 1 : 0);
    SetBit(idw, 11, 1);
}


bool
DatasetMgmt::GetIDW() const
{
    LOG_NRM("Getting IDW");
    return GetBit(11, 1);
}


void
DatasetMgmt::SetIDR(bool idr)
{
    LOG_NRM("Setting IDR = %d", idr ? 1 : 0);
    SetBit(idr, 11, 0);
}


bool
DatasetMgmt::GetIDR() const
{
    LOG_NRM("Getting IDR");
    return GetBit(11, 0);
}

