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

#include "deleteIOCQ.h"
#include "../Utils/buffers.h"

SharedDeleteIOCQPtr DeleteIOCQ::NullDeleteIOCQPtr;
const uint8_t DeleteIOCQ::Opcode = 0x04;


DeleteIOCQ::DeleteIOCQ() : Cmd(Trackable::OBJ_DELETEIOCQ)
{
    Cmd::Init(Opcode, DATADIR_NONE, 64);
}


DeleteIOCQ::~DeleteIOCQ()
{
}


void
DeleteIOCQ::Init(const SharedIOCQPtr iocq)
{
    {   // Handle DWORD 10
        uint32_t dword10 = GetDword(10);

        // Handle Q ID
        dword10 &= ~0x0000ffff;
        dword10 |= (uint32_t)iocq->GetQId();
        LOG_NRM("Init delete IOCQ cmd for CQ=%d", (uint32_t)iocq->GetQId());

        SetDword(dword10, 10);
    }   // Handle DWORD 10
}

