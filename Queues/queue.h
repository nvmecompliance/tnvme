#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "tnvme.h"
#include "trackable.h"
#include "../Singletons/memBuffer.h"


/**
* This class is the base class to all other queue classes. It is not meant to
* be instantiated. This class contains all things common to queues at a high
* level.  After instantiation by a child the Init() method must be called be
* to attain something useful.
*
* @note This class may throw exceptions.
*/
class Queue : public Trackable
{
public:
    /**
     * @param fd Pass the opened file descriptor for the device under test
     * @param objBeingCreated Pass the type of object this child class is
     * @param life Pass the lifetime of the object being create
     * @param ownByRsrcMngr Pass true if the RsrcMngr created this obj,
     *      otherwise false.
     */
    Queue(int fd, Trackable::ObjType objBeingCreated, Trackable::Lifetime life,
        bool ownByRsrcMngr);
    virtual ~Queue();

    /// A Q can either be a CQ or an SQ
    virtual bool GetIsCQ() = 0;

    /// A Q can either be of administration or IO type
    bool GetIsAdmin() { return (mQId == 0); }

    /// A Q can either be contiguous or discontiguous
    bool GetIsContig();

    uint16_t GetQId() { return mQId; }
    uint16_t GetNumEntries() { return mNumEntries; }
    uint16_t GetEntrySize() { return mEntrySize; }
    uint64_t GetQSize() { return (mEntrySize * mNumEntries); }

    /// Get the memory which encompasses the Q's contents
    uint8_t const *GetQBuffer();


protected:
    /// file descriptor to the device under test
    int mFd;

    /// mQId==0 implies admin Q
    uint16_t mQId;
    uint16_t mEntrySize;
    uint16_t mNumEntries;

    /// dnvme alloc'd Q memory which is mmap'd back into user space as RO
    uint8_t *mContigBuf;
    /// tnvme alloc'd Q memory, handed off to this obj during init'ing
    MemBuffer *mDiscontigBuf;

    /**
     * Initialize this object
     * @param qId Pass the queue's ID
     * @param entrySize Pass the number of bytes encompassing each element
     * @param numEntries Pass the number of elements within the Q
     */
    void Init(uint16_t qId, uint16_t entrySize, uint16_t numEntries);


private:
    Queue();
};


#endif
