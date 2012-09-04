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

#include "securityRcv.h"
#include "globals.h"
#include "../Utils/buffers.h"

SharedSecurityRcvPtr SecurityRcv::NullSecurityRcvPtr;
const uint8_t SecurityRcv::Opcode = 0x82;


SecurityRcv::SecurityRcv() : BaseSecurity(Trackable::OBJ_SECURITYRCV)
{
    Init(Opcode, DATADIR_FROM_DEVICE, 64);
}


SecurityRcv::~SecurityRcv()
{
}



