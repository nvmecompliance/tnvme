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

#include "baseFeatures.h"


const uint8_t BaseFeatures::FID_ARBITRATION          = 0x01;
const uint8_t BaseFeatures::FID_PWR_MGMT             = 0x02;
const uint8_t BaseFeatures::FID_LBA_RANGE            = 0x03;
const uint8_t BaseFeatures::FID_TEMP_THRESHOLD       = 0x04;
const uint8_t BaseFeatures::FID_ERR_RECOVERY         = 0x05;
const uint8_t BaseFeatures::FID_VOL_WR_CACHE         = 0x06;
const uint8_t BaseFeatures::FID_NUM_QUEUES           = 0x07;
const uint8_t BaseFeatures::FID_IRQ_COALESCING       = 0x08;
const uint8_t BaseFeatures::FID_IRQ_VEC_CONFIG       = 0x09;
const uint8_t BaseFeatures::FID_WRITE_ATOMICITY      = 0x0a;
const uint8_t BaseFeatures::FID_ASYNC_EVENT_CONFIG   = 0x0b;
const uint8_t BaseFeatures::FID_SW_PROGRESS          = 0x80;


BaseFeatures::BaseFeatures() : Cmd(Trackable::OBJTYPE_FENCE)
{
    // This constructor will throw
}


BaseFeatures::BaseFeatures(Trackable::ObjType objBeingCreated) :
    Cmd(objBeingCreated)
{
}


BaseFeatures::~BaseFeatures()
{
}


void
BaseFeatures::SetFID(uint8_t fid)
{
    LOG_NRM("Setting FID: 0x%02X", fid);
    SetByte(fid, 10, 0);
}


uint8_t
BaseFeatures::GetFID() const
{
    LOG_NRM("Getting FID");
    return GetByte(10, 0);
}

