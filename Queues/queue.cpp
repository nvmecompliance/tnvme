#include "queue.h"


Queue::Queue() :
    Trackable(Trackable::OBJTYPE_FENCE, Trackable::LIFETIME_FENCE, false)
{
    // This constructor will throw
}


Queue::Queue(int fd, Trackable::ObjType objBeingCreated,
    Trackable::Lifetime life, bool ownByRsrcMngr) :
        Trackable(objBeingCreated, life, ownByRsrcMngr)
{
    mFd = fd;
    if (mFd < 0) {
        LOG_DBG("Object created with a bad FD=%d", fd);
        return;
    }

    mQId = 0;
    mEntrySize = 0;
    mNumEntries = 0;
    mDiscontigBuf = NULL;
    mContigBuf = NULL;
}


Queue::~Queue()
{
    // Children are responsible for delete Q memory
}


bool
Queue::GetIsContig()
{
    if ((mContigBuf == NULL) && (mDiscontigBuf == NULL)) {
        LOG_DBG("Uninitialized Q");
        throw exception();
    } else if ((mContigBuf != NULL) && (mDiscontigBuf != NULL)) {
        LOG_DBG("Illegally configured Q");
        throw exception();
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
Queue::Init(uint16_t qId, uint16_t entrySize, uint16_t numEntries)
{
    if (mDiscontigBuf != NULL) {
        LOG_DBG("Obj already init'd for discontiguous parameters");
        throw exception();
    } else if (mContigBuf != NULL) {
        LOG_DBG("Obj does not support re-allocations, create a new obj");
        throw exception();
    }

    mQId = qId;
    mEntrySize = entrySize;
    mNumEntries = numEntries;
}
