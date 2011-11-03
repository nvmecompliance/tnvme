#include "memBuffer.h"
#include <stdlib.h>
#include <string.h>

SharedMemBufferPtr MemBuffer::NullMemBufferPtr;


MemBuffer::MemBuffer() : Trackable(Trackable::OBJ_MEMBUFFER)
{
    InitMemberVariables();
}


MemBuffer::~MemBuffer()
{
    // Either new or posix_memalign() was used to allocate memory
    if (mAllocByNewOperator) {
        if (mRealBaseAddr)
            delete [] mRealBaseAddr;
    } else {
        if (mRealBaseAddr)
            free(mRealBaseAddr);
    }
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
MemBuffer::Zero()
{
    if (mRealBaseAddr)
        memset(mVirBaseAddr, 0, mVirBufSize);
}


void
MemBuffer::InitOffset1stPage(uint32_t bufSize, bool initMem, uint8_t initVal,
    uint32_t offset1stPg)
{
    int err;
    uint32_t realBufSize;
    uint32_t align = sysconf(_SC_PAGESIZE);


    if (mRealBaseAddr != NULL) {
        LOG_DBG("Reallocating a previously allocated buffer not supported");
        throw exception();
    } else if (offset1stPg % sizeof(uint32_t) != 0) {
        LOG_DBG("Offset into page 1 not aligned to: 0x%02lX", sizeof(uint32_t));
        throw exception();
    }
    mAllocByNewOperator = false;  // using posix_memalign()

    // All memory is allocated page aligned, offsets into the 1st page requires
    // asking for more memory than the caller desires and then tracking the
    // virtual pointer into the real allocation as a side affect.
    mVirBufSize = bufSize;
    realBufSize = (bufSize + offset1stPg);
    err = posix_memalign((void **)&mRealBaseAddr, align, realBufSize);
    if (err) {
        LOG_DBG("Memory allocation failed with error code: 0x%02X", err);
        InitMemberVariables();
        throw exception();
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
MemBuffer::InitAlignment(uint32_t bufSize, bool initMem, uint8_t initVal,
    uint32_t align)
{
    int err;


    if (mRealBaseAddr != NULL) {
        LOG_DBG("Reallocating a previously allocated buffer not supported");
        throw exception();
    } else if (align % sizeof(void *) != 0) {
        LOG_DBG("Requested alignment not aligned to: 0x%02lX", sizeof(void *));
        throw exception();
    }
    mAllocByNewOperator = false;  // using posix_memalign()

    mVirBufSize = bufSize;
    err = posix_memalign((void **)&mRealBaseAddr, align, mVirBufSize);
    if (err) {
        LOG_DBG("Memory allocation failed with error code: 0x%02X", err);
        InitMemberVariables();
        throw exception();
    }
    mVirBaseAddr = mRealBaseAddr;
    mAlignment = align;

    if (initMem)
        memset(mVirBaseAddr, initVal, mVirBufSize);
}


void
MemBuffer::Init(uint32_t bufSize, bool initMem, uint8_t initVal)
{
    if (mRealBaseAddr != NULL) {
        LOG_DBG("Reallocating a previously allocated buffer not supported");
        throw exception();
    }
    mAllocByNewOperator = true;

    mVirBufSize = bufSize;
    mRealBaseAddr = new (nothrow) uint8_t[mVirBufSize];
    if (mRealBaseAddr == NULL) {
        LOG_DBG("Memory allocation failed");
        InitMemberVariables();
        throw exception();
    }
    mVirBaseAddr = mRealBaseAddr;
    mAlignment = 0;

    if (initMem)
        memset(mVirBaseAddr, initVal, mVirBufSize);
}
