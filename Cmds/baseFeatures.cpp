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
#include "featureDefs.h"

#define ZZ(a,b)  b,
const uint8_t FID[] = {
    FEATURE_TABLE
    FID_FENCE    // always must be the last element
};
#undef ZZ


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

