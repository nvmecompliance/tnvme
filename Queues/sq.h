#ifndef _SQ_H_
#define _SQ_H_

#include "queue.h"


/**
* This class extends the base class. It is also not meant to be instantiated.
* This class contains all things common to SQ's at a high level. After
* instantiation by a child the Init() methods must be called to attain
* something useful.
*
* @note This class may throw exceptions.
*/
class SQ : public Queue
{
public:
    SQ(int fd, Trackable::ObjType objBeingCreated, Trackable::Lifetime life,
        bool ownByRsrcMngr);
    virtual ~SQ();

    virtual bool GetIsCQ() { return false; }

    struct nvme_gen_sq GetQMetrics();

    /// All SQ's have an associated CQ to where its completions will be placed
    uint16_t GetCqId() { return mCqId; }

    uint8_t GetPriority() { return mPriority; }


protected:
    /**
     * Initialize this object and allocates contiguous Q content memory.
     * @param qId Pass the queue's ID
     * @param entrySize Pass the number of bytes encompassing each element
     * @param numEntries Pass the number of elements within the Q
     * @param cqId Pass the assoc CQ ID to which this SQ will be associated
     * @param priority Pass this Q's priority value, must be a 2 bit value
     */
    void Init(uint16_t qId, uint16_t entrySize, uint16_t numEntries,
        uint16_t cqId, uint8_t priority);

    /**
     * Initialize this object and allocates discontiguous Q content memory.
     * @param qId Pass the queue's ID
     * @param entrySize Pass the number of bytes encompassing each element
     * @param numEntries Pass the number of elements within the Q
     * @param memBuffer Hand off a buffer which must satisfy
     *        MemBuffer.GetBufSize()>=(numEntries * entrySize). It must have
     *        the same life span as this object, it must have been created
     *        by the same means as this object, and must only ever be accessed
     *        as RO. Writing to this buffer will have unpredictable results.
     *        It will also become owned by this object, it won't have to be
     *        explicitly deleted when this object goes out of scope.
     * @param cqId Pass the assoc CQ ID to which this SQ will be associated
     * @param priority Pass this Q's priority value, must be a 2 bit value
     */
    void Init(uint16_t qId, uint16_t entrySize, uint16_t numEntries,
        MemBuffer &memBuffer, uint16_t cqId, uint8_t priority);


private:
    SQ();

    const uint64_t MMAP_QTYPE_BITMASK;
    uint16_t mCqId;
    uint8_t mPriority;

    /**
     * Create an IOSQ
     * @param q Pass the IOSQ's definition
     */
    void CreateIOSQ(struct nvme_prep_sq &q);
};


#endif
