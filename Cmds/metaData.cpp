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

#include <string.h>
#include "metaData.h"
#include "globals.h"
#include "../Utils/buffers.h"
#include "../Exception/frmwkEx.h"

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
    if (GetDataDir() == DATADIR_NONE)
        throw FrmwkEx(HERE, "Meta buffer assoc, but no data direction");

    if (gRsrcMngr->ReserveMetaBuf(mMetaData) == false)
        throw FrmwkEx(HERE, "Meta data alloc request denied");
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
MetaData::Dump(DumpFilename filename, string fileHdr) const
{
    Buffers::Dump(filename, mMetaData.buf, 0, ULONG_MAX, GetMetaBufferSize(),
        fileHdr);
}


void
MetaData::SetMetaDataPattern(DataPattern dataPat, uint64_t initVal,
    uint32_t offset, uint32_t length)
{
    LOG_NRM("Write data pattern: initial value = 0x%016llX",
        (long long unsigned int)initVal);

    if (GetMetaBuffer() == NULL)
        return;

    length = (length == UINT32_MAX) ? GetMetaBufferSize() : length;
    if ((length + offset) > GetMetaBufferSize())
        throw FrmwkEx(HERE, "Length exceeds total meta buffer allocated size");

    switch (dataPat)
    {
    case DATAPAT_CONST_8BIT:
        {
            LOG_NRM("Write data pattern: constant 8 bit");
            uint8_t *rawPtr = GetMetaBuffer() + offset;
            for (uint64_t i = 0; i < length; i++)
                *rawPtr++ = (uint8_t)initVal;
        }
        break;

    case DATAPAT_CONST_16BIT:
        {
            LOG_NRM("Write data pattern: constant 16 bit");
            uint16_t *rawPtr = (uint16_t *)(GetMetaBuffer() + offset);
            for (uint64_t i = 0; i < (length / sizeof(uint16_t)); i++)
                *rawPtr++ = (uint16_t)initVal;
        }
        break;

    case DATAPAT_CONST_32BIT:
        {
            LOG_NRM("Write data pattern: constant 32 bit");
            uint32_t *rawPtr = (uint32_t *)(GetMetaBuffer() + offset);
            for (uint64_t i = 0; i < (length / sizeof(uint32_t)); i++)
                *rawPtr++ = (uint32_t)initVal;
        }
        break;

    case DATAPAT_INC_8BIT:
        {
            LOG_NRM("Write data pattern: incrementing 8 bit");
            uint8_t *rawPtr = GetMetaBuffer() + offset;
            for (uint64_t i = 0; i < length; i++)
                *rawPtr++ = (uint8_t)initVal++;
        }
        break;

    case DATAPAT_INC_16BIT:
        {
            LOG_NRM("Write data pattern: incrementing 16 bit");
            uint16_t *rawPtr = (uint16_t *)(GetMetaBuffer() + offset);
            for (uint64_t i = 0; i < (length / sizeof(uint16_t)); i++)
                *rawPtr++ = (uint16_t)initVal++;
        }
        break;

    case DATAPAT_INC_32BIT:
        {
            LOG_NRM("Write data pattern: incrementing 32 bit");
            uint32_t *rawPtr = (uint32_t *)(GetMetaBuffer() + offset);
            for (uint64_t i = 0; i < (length / sizeof(uint32_t)); i++)
                *rawPtr++ = (uint32_t)initVal++;
        }
        break;

    default:
        throw FrmwkEx(HERE, "Unsupported data pattern %d", dataPat);
    }
}


bool
MetaData::CompareMetaBuffer(SharedMemBufferPtr compTo)
{
    if (compTo->GetBufSize() > GetMetaBufferSize()) {
        throw FrmwkEx(HERE, "Compare buffer size > max meta buff size: %d > %d",
            compTo->GetBufSize(), GetMetaBufferSize());
    }

    if (memcmp(compTo->GetBuffer(), GetMetaBuffer(),
        compTo->GetBufSize()) != 0) {
        LOG_ERR("Detected data miscompare.");
        return false;
    }
    return true;
}
