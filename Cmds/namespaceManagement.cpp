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

#include "namespaceManagement.h"


SharedNamespaceManagementPtr NamespaceManagement::NullNamespaceManagementPtr;
const uint8_t NamespaceManagement::Opcode = 0x0D;


NamespaceManagement::NamespaceManagement() : Cmd(Trackable::OBJ_NAMESPACEMANAGEMENT)
{
    Init(Opcode, DATADIR_TO_DEVICE, 64); //64B size of buffer used to hold the admin command 16DW

    // No cmd should ever be created which violates these masking possibilities
    send_64b_bitmask allowPrpMask = (send_64b_bitmask) (MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    SetPrpAllowed(allowPrpMask);
}

NamespaceManagement::~NamespaceManagement()
{
}

void
NamespaceManagement::SetSEL(uint8_t sel)
{
    LOG_NRM("Setting SEL = 0x%01X", sel);
    uint8_t val = GetByte(10, 0);
    SetByte(val | (sel & 0xF), 10, 0); // 1st byte in DW10
}


uint8_t
NamespaceManagement::GetSEL() const
{
    LOG_NRM("Getting SEL");
    return GetByte(10, 0) & 0xF;
}

