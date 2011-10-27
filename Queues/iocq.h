#ifndef _IOCQ_H_
#define _IOCQ_H_

#include "cq.h"


/**
* This class is meant to be instantiated and represents an IOCQ. After
* instantiation the Init() methods must be called to attain something useful.
*
* @note This class may throw exceptions.
*/
class IOCQ : public CQ
{
public:
    /**
     * @param fd Pass the opened file descriptor for the device under test
     * @param life Pass the lifetime of the object being created
     * @param ownByRsrcMngr Pass true if the RsrcMngr created this obj,
     *      otherwise false.
     */
    IOCQ(int fd, Trackable::Lifetime life, bool ownByRsrcMngr);
    virtual ~IOCQ();

    /**
     * Initialize this object and allocates contiguous Q content memory.
     * @param qId Pass the queue's ID
     * @param numEntries Pass the number of elements within the Q
     * @param irqEnabled Pass true if IRQ's are to be enabled for this Q
     * @param irqVec If (irqEnabled==true) then what the IRQ's vector
     */
    void Init(uint16_t qId, uint16_t numEntries, bool irqEnabled,
        uint16_t irqVec);

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
     * @param irqEnabled Pass true if IRQ's are to be enabled for this Q
     * @param irqVec If (irqEnabled==true) then what the IRQ's vector
     */
    void Init(uint16_t qId, uint16_t numEntries, MemBuffer &memBuffer,
        bool irqEnabled, uint16_t irqVec);


private:
    IOCQ();
};


#endif
