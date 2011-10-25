#include "memBuffer.h"
#include <stdlib.h>
#include <string.h>


MemBuffer::MemBuffer() :
    Trackable(Trackable::OBJTYPE_FENCE, Trackable::LIFETIME_FENCE, false)
{
    // This constructor will throw
}


MemBuffer::MemBuffer(Trackable::Lifetime life, bool ownByRsrcMngr) :
    Trackable(Trackable::OBJ_MEMBUFFER, life, ownByRsrcMngr)
{
    InitMemberVariables();
}


MemBuffer::~MemBuffer()
{
    if (mRealBaseAddr)
        free(mRealBaseAddr);
}


void
MemBuffer::InitMemberVariables()
{
    mRealBaseAddr = NULL;
    mVirBaseAddr = NULL;
    mVirBufSize = 0;
    mAlignment = 0;
}


void
MemBuffer::Reset()
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
