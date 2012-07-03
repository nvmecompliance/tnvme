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

#include "fwImgDnld.h"

SharedFWImgDnldPtr FWImgDnld::NullFWImgDnldPtr;
const uint8_t FWImgDnld::Opcode = 0x11;


FWImgDnld::FWImgDnld() : Cmd(Trackable::OBJ_FWIMGDNLD)
{
    Init(Opcode, DATADIR_TO_DEVICE, 64);

    // No cmd should ever be created which violates these masking possibilities
    send_64b_bitmask allowPrpMask = (send_64b_bitmask)
        (MASK_PRP1_PAGE | MASK_PRP2_PAGE | MASK_PRP2_LIST);
    SetPrpAllowed(allowPrpMask);
}


FWImgDnld::~FWImgDnld()
{
}


void
FWImgDnld::SetNUMD(uint32_t numd)
{
    LOG_NRM("Setting NUMD = 0x%04X", numd);
    SetDword(numd, 10);
}


uint32_t
FWImgDnld::GetNUMD() const
{
    LOG_NRM("Getting NUMD");
    return GetDword(10);
}


void
FWImgDnld::SetOFST(uint32_t ofst)
{
    LOG_NRM("Setting OFST = 0x%04X", ofst);
    SetDword(ofst, 11);
}


uint32_t
FWImgDnld::GetOFST() const
{
    LOG_NRM("Getting OFST");
    return GetDword(11);
}


