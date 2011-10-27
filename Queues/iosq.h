#ifndef _IOSQ_H_
#define _IOSQ_H_

#include "sq.h"


/**
* This class is meant to be instantiated and represents an IOSQ. After
* instantiation the Init() methods must be called to attain something useful.
*
* @note This class may throw exceptions.
*/
class IOSQ : public SQ
{
public:
    /**
     * @param fd Pass the opened file descriptor for the device under test
     * @param life Pass the lifetime of the object being created
     * @param ownByRsrcMngr Pass true if the RsrcMngr created this obj,
     *      otherwise false.
     */
    IOSQ(int fd, Trackable::Lifetime life, bool ownByRsrcMngr);
    virtual ~IOSQ();

    uint8_t GetPriority() { return mPriority; }

    /**
     * Initialize this object and allocates contiguous Q content memory.
     * @param qId Pass the queue's ID
     * @param numEntries Pass the number of elements within the Q
     * @param cqId Pass the assoc CQ ID to which this SQ will be associated
     * @param priority Pass this Q's priority value, must be a 2 bit value
     */
    void Init(uint16_t qId, uint16_t numEntries, uint16_t cqId,
        uint8_t priority);

    /**
     * Initialize this object and allocates discontiguous Q content memory.
     * @param qId Pass the queue's ID
     * @param numEntries Pass the number of elements within the Q
     * @param memBuffer Hand off a buffer which must satisfy
     *        MemBuffer.GetBufSize()>=(numEntries * entrySize). It must have
     *        the same life span as this object, it must have been created
     *        by the same means as this object, and must only ever be accessed
     *        as RO. Writing to this buffer will have unpredictable results.
     *        It will also become owned by this object, it won't have to be
     *        explicitly deleted when this object goes out of scope.
     *        @param cqId Pass the assoc CQ ID to which this SQ will be associated
     * @param priority Pass this Q's priority value, must be a 2 bit value
     */
    void Init(uint16_t qId, uint16_t numEntries, MemBuffer &memBuffer,
        uint16_t cqId, uint8_t priority);


private:
    IOSQ();

    uint8_t mPriority;
};


#endif
