#ifndef _IOCQ_H_
#define _IOCQ_H_

#include "cq.h"

class IOCQ;    // forward definition
typedef boost::shared_ptr<IOCQ>        SharedIOCQPtr;
#define CAST_TO_IOCQ(shared_trackable_ptr)  \
        boost::shared_polymorphic_downcast<IOCQ>(shared_trackable_ptr);


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
     */
    IOCQ(int fd);
    virtual ~IOCQ();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedIOCQPtr NullIOCQPtr;

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
     * @param memBuffer Hand off this Q's memory. It must satisfy
     *      MemBuffer.GetBufSize()>=(numEntries * gCtrlrConfig->GetIOCQES(). It
     *      must only ever be accessed as RO. Writing to this buffer will have
     *      unpredictable results.
     * @param irqEnabled Pass true if IRQ's are to be enabled for this Q
     * @param irqVec If (irqEnabled==true) then what the IRQ's vector
     */
    void Init(uint16_t qId, uint16_t numEntries,
        const SharedMemBufferPtr memBuffer, bool irqEnabled, uint16_t irqVec);


private:
    IOCQ();
};


#endif
