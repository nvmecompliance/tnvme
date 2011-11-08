#ifndef _CQ_H_
#define _CQ_H_

#include "queue.h"
#include "ce.h"


/**
* This class extends the base class. It is also not meant to be instantiated.
* This class contains all things common to CQ's at a high level. After
* instantiation by a child the Init() methods must be called to attain
* something useful.
*
* @note This class may throw exceptions.
*/
class CQ : public Queue
{
public:
    /**
     * @param fd Pass the opened file descriptor for the device under test
     * @param objBeingCreated Pass the type of object this child class is
     */
    CQ(int fd, Trackable::ObjType objBeingCreated);
    virtual ~CQ();

    virtual bool GetIsCQ() { return true; }

    struct nvme_gen_cq GetQMetrics();

    bool GetIrqEnabled() { return mIrqEnabled; }
    uint16_t GetIrqVector() { return mIrqVec; }

    /**
     * Get a Completion Element (CE) at CQ position indicated by indexPtr.
     * @param indexPtr Pass [0 to (GetNumEntries()-1)] as the index into the CQ.
     * @return The CE requested.
     */
    union CE GetCE(uint16_t indexPtr);


protected:
    /**
     * Initialize this object and allocates contiguous Q content memory.
     * @param qId Pass the queue's ID
     * @param entrySize Pass the number of bytes encompassing each element
     * @param numEntries Pass the number of elements within the Q
     * @param irqEnabled Pass true if IRQ's are to be enabled for this Q
     * @param irqVec if (irqEnabled==true) then what the IRQ's vector
     */
    void Init(uint16_t qId, uint16_t entrySize, uint16_t numEntries,
        bool irqEnabled, uint16_t irqVec);

    /**
     * Initialize this object and allocates discontiguous Q content memory.
     * @param qId Pass the queue's ID
     * @param entrySize Pass the number of bytes encompassing each element
     * @param numEntries Pass the number of elements within the Q
     * @param memBuffer Hand off this Q's memory. It must satisfy
     *      MemBuffer.GetBufSize()>=(numEntries * entrySize). It must only ever
     *      be accessed as RO. Writing to this buffer will have unpredictable
     *      results.
     * @param irqEnabled Pass true if IRQ's are to be enabled for this Q
     * @param irqVec if (irqEnabled==true) then what the IRQ's vector
     */
    void Init(uint16_t qId, uint16_t entrySize, uint16_t numEntries,
        const SharedMemBufferPtr memBuffer, bool irqEnabled, uint16_t irqVec);


private:
    CQ();

    bool mIrqEnabled;
    uint16_t mIrqVec;

    /**
     * Create an IOCQ
     * @param q Pass the IOCQ's definition
     */
    void CreateIOCQ(struct nvme_prep_cq &q);
};


#endif
