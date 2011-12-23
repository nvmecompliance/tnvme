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

#include "createIOSQ.h"
#include "../Utils/buffers.h"


CreateIOSQ::CreateIOSQ() :
    AdminCmd(0, Trackable::OBJTYPE_FENCE)
{
    // This constructor will throw
}


CreateIOSQ::CreateIOSQ(int fd) :
    AdminCmd(fd, Trackable::OBJ_CREATEIOSQ)
{
    AdminCmd::Init(0x01, DATADIR_TO_DEVICE);
}


CreateIOSQ::~CreateIOSQ()
{
}


void
CreateIOSQ::Init(const SharedIOSQPtr iosq)
{
    // Setup the PRP buffer based upon contig or non-contig memory
    int prpField = MASK_PRP1_PAGE;
    if (iosq->GetIsContig() == false)
        prpField |= MASK_PRP1_LIST;
    SetPrpBuffer((send_64b_bitmask)prpField, iosq->GetQBuffer(),
        iosq->GetQSize());

    {   // Handle DWORD 10
        uint32_t dword10 = GetDword(10);

        // Handle q size
        dword10 &= ~0xffff0000;
        dword10 |= (((uint32_t)iosq->GetNumEntries()) << 16);

        // Handle Q ID
        dword10 &= ~0x0000ffff;
        dword10 |= (uint32_t)iosq->GetQId();

        SetDword(dword10, 10);
    }   // Handle DWORD 10

    {   // Handle DWORD 11
        uint32_t dword11 = GetDword(11);

        // Handle the PC bit
        if (iosq->GetIsContig())
            dword11 |= 0x00000001;
        else
            dword11 &= ~0x00000001;

        // Handle Q priority
        dword11 &= ~0x00000006;
        dword11 |= (((uint32_t)iosq->GetPriority()) << 1);

        // Handle CQ ID
        dword11 &= ~0xffff0000;
        dword11 |= iosq->GetCqId();

        SetDword(dword11, 11);
    }   // Handle DWORD 11
}


void
CreateIOSQ::Dump(LogFilename filename, string fileHdr) const
{
    Cmd::Dump(filename, fileHdr);
    PrpData::Dump(filename, "Payload contents:");
    MetaData::Dump(filename, "Meta data contents:");
}
