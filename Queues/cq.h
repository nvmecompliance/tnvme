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
     * Peek at a Completion Element (CE) at CQ position indicated by indexPtr.
     * Only dnvme can reap CE's from a CQ by calling Reap(), however user space
     * does have eyes into that CQ's memory, and thus allows peeking at any CE
     * at any time without reaping anything.
     * @param indexPtr Pass [0 to (GetNumEntries()-1)] as the index into the CQ.
     * @return The CE requested.
     */
    union CE PeekCE(uint16_t indexPtr);

    /**
     * Dump the entire contents of CE at CQ position indicated by indexPtr to
     * the logging endpoint. Similar to PeekCE() but logs the CE instead.
     * @param indexPtr Pass the index into the Q for which element to log
     */
    void LogCE(uint16_t indexPtr);

    /**
     * Inquire as to the number of CE's which are present in this CQ. Returns
     * immediately, does not block.
     * @return The number of unreap'd CE's awaiting
     */
    uint16_t ReapInquiry();

    /**
     * Inquire as to the number of CE's which are present in this CQ. If the
     * number of CE's are 0, then a wait period is entered until such time
     * a CE arrives or a timeout period expires.
     * @param ms Pass the max number of milliseconds to wait until CE's arrive.
     * @param numCE Returns the number of unreap'd CE's awaiting
     * @return true when CE's are awaiting to be reaped, otherwise a timeout
     */
    bool ReapInquiryWaitAny(uint16_t ms, uint16_t &numCE);

    /**
     * Wait until at least the specified number of CE's become available or
     * until a time out period expires.
     * @param ms Pass the max number of milliseconds to wait until numTil CE's
     *      arrive.
     * @param numTil Pass the number of CE's that need to become available
     * @param numCE Returns the number of unreap'd CE's awaiting
     * @return true when CE's are awaiting to be reaped, otherwise a timeout
     */
    bool ReapInquiryWaitSpecify(uint16_t ms, uint16_t numTil, uint16_t &numCE);

    /**
     * Reap a specified number of Completion Elements (CE) from this CQ. The
     * memBuffer will be resized. Calling this method when (ReapInquiry() == 0)
     * is fine.
     * @param ceRemain Returns the number of CE's left in the CQ after reaping
     * @param memBuffer Pass a buffer to contain the CE's requested. The
     *      contents of the buffer will be lost and the buffer will be resized
     *      to fulfill ceDesire.
     * @param ceDesire Pass the number of CE's desired to be reaped, 0 indicates
     *      reap all which can be reaped.
     * @param zeroMem Pass true to zero out memBuffer before reaping, otherwise
     *      the buffer is not modified.
     * @return Returns the actual number of CE's reaped
     */
    uint16_t Reap(uint16_t &ceRemain, SharedMemBufferPtr memBuffer,
        uint16_t ceDesire = 0, bool zeroMem = false);


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
