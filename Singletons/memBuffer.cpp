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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "memBuffer.h"
#include "../Utils/buffers.h"
#include "../Exception/frmwkEx.h"

SharedMemBufferPtr MemBuffer::NullMemBufferPtr;


MemBuffer::MemBuffer() : Trackable(Trackable::OBJ_MEMBUFFER)
{
    InitMemberVariables();
}


MemBuffer::~MemBuffer()
{
    DeallocateResources();
}


void
MemBuffer::InitMemberVariables()
{
    mAllocByNewOperator = true;
    mRealBaseAddr = NULL;
    mVirBaseAddr = NULL;
    mVirBufSize = 0;
    mAlignment = 0;
}


void
MemBuffer::DeallocateResources()
{
    // Either new or posix_memalign() was used to allocate memory
    if (mAllocByNewOperator) {
        if (mRealBaseAddr)
            delete [] mRealBaseAddr;
    } else {
        if (mRealBaseAddr)
            free(mRealBaseAddr);
    }
    InitMemberVariables();
}


void
MemBuffer::InitOffset1stPage(uint32_t bufSize, uint32_t offset1stPg,
    bool initMem, uint8_t initVal)
{
    int err;
    uint32_t realBufSize;
    uint32_t align = sysconf(_SC_PAGESIZE);


    LOG_NRM(
        "Init buffer; size: 0x%08X, offset: 0x%08X, init: %d, value: 0x%02X",
        bufSize, offset1stPg, initMem, initVal);
    if (offset1stPg % sizeof(uint32_t) != 0) {
        throw FrmwkEx("Offset into page 1 not aligned to: 0x%02lX",
            sizeof(uint32_t));
    }

    // Support resizing/reallocation
    if (mRealBaseAddr != NULL)
        DeallocateResources();
    mAllocByNewOperator = false;  // using posix_memalign()

    // All memory is allocated page aligned, offsets into the 1st page requires
    // asking for more memory than the caller desires and then tracking the
    // virtual pointer into the real allocation as a side affect.
    mVirBufSize = bufSize;
    realBufSize = (bufSize + offset1stPg);
    err = posix_memalign((void **)&mRealBaseAddr, align, realBufSize);
    if (err) {
        InitMemberVariables();
        throw FrmwkEx("Memory allocation failed with error code: 0x%02X",
            err);
    }
    mVirBaseAddr = (mRealBaseAddr + offset1stPg);
    if (offset1stPg)
        mAlignment = offset1stPg;
    else
        mAlignment = align;

    if (initMem)
        memset(mVirBaseAddr, initVal, mVirBufSize);
}


void
MemBuffer::InitAlignment(uint32_t bufSize, uint32_t align, bool initMem,
    uint8_t initVal)
{
    int err;

    LOG_NRM("Init buffer; size: 0x%08X, align: 0x%08X, init: %d, value: 0x%02X",
        bufSize, align, initMem, initVal);
    if (align % sizeof(void *) != 0) {
        throw FrmwkEx("Req'd alignment 0x%08X, is not modulo 0x%02lX",
            align, sizeof(void *));
    }

    // Support resizing/reallocation
    if (mRealBaseAddr != NULL)
        DeallocateResources();
    mAllocByNewOperator = false;  // using posix_memalign()

    mVirBufSize = bufSize;
    err = posix_memalign((void **)&mRealBaseAddr, align, mVirBufSize);
    if (err) {
        InitMemberVariables();
        throw FrmwkEx("Memory allocation failed with error code: 0x%02X",
            err);
    }
    mVirBaseAddr = mRealBaseAddr;
    mAlignment = align;

    if (initMem)
        memset(mVirBaseAddr, initVal, mVirBufSize);
}


void
MemBuffer::Init(uint32_t bufSize, bool initMem, uint8_t initVal)
{
    LOG_NRM("Init buffer; size: 0x%08X, init: %d, value: 0x%02X",
        bufSize, initMem, initVal);

    // Support resizing/reallocation
    if (mRealBaseAddr != NULL)
        DeallocateResources();
    mAllocByNewOperator = true;

    mVirBufSize = bufSize;
    mRealBaseAddr = new (nothrow) uint8_t[mVirBufSize];
    if (mRealBaseAddr == NULL) {
        InitMemberVariables();
        throw FrmwkEx("Memory allocation failed");
    }
    mVirBaseAddr = mRealBaseAddr;
    mAlignment = 0;

    if (initMem)
        memset(mVirBaseAddr, initVal, mVirBufSize);
}


void
MemBuffer::Log(uint32_t bufOffset, unsigned long length)
{
    Buffers::Log(GetBuffer(), bufOffset, length, GetBufSize(), "MemBuffer");
}


void
MemBuffer::Dump(LogFilename filename, string fileHdr)
{
    Buffers::Dump(filename, GetBuffer(), 0, ULONG_MAX, GetBufSize(), fileHdr);

}


void
MemBuffer::SetDataPattern(DataPattern dataPat, uint64_t initVal)
{
    LOG_NRM("Write data pattern: initial value = 0x%016llX",
        (long long unsigned int)initVal);

    if (mRealBaseAddr == NULL)
        return;

    switch (dataPat)
    {
    case DATAPAT_CONST_8BIT:
        {
            LOG_NRM("Write data pattern: constant 8 bit");
            memset(mVirBaseAddr, (uint8_t)initVal, mVirBufSize);
        }
        break;

    case DATAPAT_CONST_16BIT:
        {
            LOG_NRM("Write data pattern: constant 16 bit");
            uint16_t *rawPtr = (uint16_t *)GetBuffer();
            for (uint64_t i = 0; (GetBufSize() / sizeof(uint16_t)); i++)
                *rawPtr++ = (uint16_t)initVal;
        }
        break;

    case DATAPAT_CONST_32BIT:
        {
            LOG_NRM("Write data pattern: constant 32 bit");
            uint32_t *rawPtr = (uint32_t *)GetBuffer();
            for (uint64_t i = 0; (GetBufSize() / sizeof(uint32_t)); i++)
                *rawPtr++ = (uint32_t)initVal;
        }
        break;

    case DATAPAT_INC_8BIT:
        {
            LOG_NRM("Write data pattern: incrementing 8 bit");
            uint8_t *rawPtr = (uint8_t *)GetBuffer();
            for (uint64_t i = 0; (GetBufSize() / sizeof(uint8_t)); i++)
                *rawPtr++ = (uint8_t)initVal++;
        }
        break;

    case DATAPAT_INC_16BIT:
        {
            LOG_NRM("Write data pattern: incrementing 16 bit");
            uint16_t *rawPtr = (uint16_t *)GetBuffer();
            for (uint64_t i = 0; i < (GetBufSize() / sizeof(uint16_t)); i++)
                *rawPtr++ = (uint16_t)initVal++;
        }
        break;

    case DATAPAT_INC_32BIT:
        {
            LOG_NRM("Write data pattern: incrementing 32 bit");
            uint32_t *rawPtr = (uint32_t *)GetBuffer();
            for (uint64_t i = 0; (GetBufSize() / sizeof(uint32_t)); i++)
                *rawPtr++ = (uint32_t)initVal++;
        }
        break;

    default:
        throw FrmwkEx("Unsupported data pattern %d", dataPat);
    }
}


bool
MemBuffer::Compare(SharedMemBufferPtr compTo)
{
    if (compTo->GetBufSize() != GetBufSize()) {
        throw FrmwkEx("Compare buffers not same size: %d != %d",
            compTo->GetBufSize(), GetBufSize());
    }

    if (memcmp(compTo->GetBuffer(), GetBuffer(), GetBufSize()) != 0) {
        LOG_ERR("Detected data miscompare");
        return false;
    }
    return true;
}
