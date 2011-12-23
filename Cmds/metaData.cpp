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

#include "metaData.h"
#include "globals.h"
#include "../Utils/buffers.h"

using namespace std;


MetaData::MetaData()
{
}


MetaData::~MetaData()
{
    gRsrcMngr->ReleaseMetaBuf(mMetaData);
}


void
MetaData::AllocMetaBuffer()
{
    if (gRsrcMngr->ReserveMetaBuf(mMetaData) == false) {
        LOG_ERR("Meta data alloc request denied");
        throw exception();
    }
}


send_64b_bitmask
MetaData::GetMetaBitmask() const
{
    // If its still a default object then nothing has allocated a meta data buf
    if (mMetaData == MetaDataBuf())
        return (send_64b_bitmask)0;
    return MASK_MPTR;
}


void
MetaData::Dump(LogFilename filename, string fileHdr) const
{
    Buffers::Dump(filename, mMetaData.buf, 0, ULONG_MAX, GetMetaBufferSize(),
        fileHdr);
}
