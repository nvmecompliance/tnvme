#ifndef _SQ_H_
#define _SQ_H_

#include "queue.h"
#include "se.h"
#include "../Cmds/cmd.h"


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
    /**
     * @param fd Pass the opened file descriptor for the device under test
     * @param objBeingCreated Pass the type of object this child class is
     */
    SQ(int fd, Trackable::ObjType objBeingCreated);
    virtual ~SQ();

    virtual bool GetIsCQ() { return false; }

    struct nvme_gen_sq GetQMetrics();

    /// All SQ's have an associated CQ to where its completions will be placed
    uint16_t GetCqId() { return mCqId; }

    /**
     * Get a Submission Element (SE) at SQ position indicated by indexPtr.
     * @param indexPtr Pass [0 to (GetNumEntries()-1)] as the index into the SQ.
     * @return The SE requested.
     */
    union SE GetSE(uint16_t indexPtr);

    /**
     * Issue the specified cmd to this queue, but does not ring any doorbell.
     * @param cmd Pass the cmd to send to this queue.
     */
    void Send(SharedCmdPtr cmd);

    /**
     * Ring the doorbell assoc with this SQ. This will commit to hardware all
     * prior cmds which were sent via Send().
     */
    void Ring();


protected:
    /**
     * Initialize this object and allocates contiguous Q content memory.
     * @param qId Pass the queue's ID
     * @param entrySize Pass the number of bytes encompassing each element
     * @param numEntries Pass the number of elements within the Q
     * @param cqId Pass the assoc CQ ID to which this SQ will be associated
     */
    void Init(uint16_t qId, uint16_t entrySize, uint16_t numEntries,
        uint16_t cqId);

    /**
     * Initialize this object and allocates discontiguous Q content memory.
     * @param qId Pass the queue's ID
     * @param entrySize Pass the number of bytes encompassing each element
     * @param numEntries Pass the number of elements within the Q
     * @param memBuffer Hand off this Q's memory. It must satisfy
     *      MemBuffer.GetBufSize()>=(numEntries * entrySize). It must only ever
     *      be accessed as RO. Writing to this buffer will have unpredictable
     *      results.
     * @param cqId Pass the assoc CQ ID to which this SQ will be associated
     */
    void Init(uint16_t qId, uint16_t entrySize, uint16_t numEntries,
        const SharedMemBufferPtr memBuffer, uint16_t cqId);


private:
    SQ();

    uint16_t mCqId;

    /**
     * Create an IOSQ
     * @param q Pass the IOSQ's definition
     */
    void CreateIOSQ(struct nvme_prep_sq &q);
};


#endif
