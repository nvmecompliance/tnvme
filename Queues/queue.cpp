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

#include "queue.h"
#include "../Utils/buffers.h"


Queue::Queue() : Trackable(Trackable::OBJTYPE_FENCE)
{
    // This constructor will throw
}


Queue::Queue(int fd, Trackable::ObjType objBeingCreated) :
    Trackable(objBeingCreated)
{
    mFd = fd;
    if (mFd < 0)
        throw FrmwkEx(HERE, "Object created with a bad FD=%d", fd);

    mQId = 0;
    mEntrySize = 0;
    mNumEntries = 0;
    mDiscontigBuf = MemBuffer::NullMemBufferPtr;
    mContigBuf = NULL;
}


Queue::~Queue()
{
    // Children are responsible for delete Q memory
}


bool
Queue::GetIsContig()
{
    if ((mContigBuf == NULL) &&
        (mDiscontigBuf == MemBuffer::NullMemBufferPtr)) {

        throw FrmwkEx(HERE, "Detected an uninitialized Q");
    } else if ((mContigBuf != NULL) &&
        (mDiscontigBuf != MemBuffer::NullMemBufferPtr)) {

        throw FrmwkEx(HERE, "Detected an illegally configured Q");
    }
    return (mContigBuf != NULL);
}


uint8_t const *
Queue::GetQBuffer()
{
    if (GetIsContig())
        return mContigBuf;
    else
        return mDiscontigBuf->GetBuffer();
}


void
Queue::Init(uint16_t qId, uint16_t entrySize, uint32_t numEntries)
{
    if (mDiscontigBuf != MemBuffer::NullMemBufferPtr) {
        throw FrmwkEx(HERE, "Obj already init'd for discontiguous parameters");
    } else if (mContigBuf != NULL) {
        throw FrmwkEx(HERE, 
            "Obj does not support re-allocations, create a new obj");
    }

    mQId = qId;
    mEntrySize = entrySize;
    mNumEntries = numEntries;
}


void
Queue::Log(uint32_t bufOffset, unsigned long length)
{
    Buffers::Log(GetQBuffer(), bufOffset, length, GetQSize(), "Queue");
}


void
Queue::Dump(DumpFilename filename, string fileHdr)
{
    Buffers::Dump(filename, GetQBuffer(), 0, ULONG_MAX, GetQSize(), fileHdr);
}
