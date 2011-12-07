/*
 * Copyright (c) 2011, Intel Corporation.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

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
     * Peek at a Submission Element (CE) at SQ position indicated by indexPtr.
     * Only dnvme can issue SE's into a SQ by calling Send(), however user space
     * does have eyes into that SQ's memory, and thus allows peeking at any SE
     * at any time without reaping anything.
     * @param indexPtr Pass [0 to (GetNumEntries()-1)] as the index into the SQ.
     * @return The SE requested.
     */
    union SE PeekSE(uint16_t indexPtr);

    /**
     * Dump the entire contents of SE at SQ position indicated by indexPtr to
     * the logging endpoint. Similar to PeekSE() but logs the SE instead.
     * param indexPtr Pass the index into the Q for which element to log
     */
    void LogSE(uint16_t indexPtr);

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
