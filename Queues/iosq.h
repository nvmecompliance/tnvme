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

#ifndef _IOSQ_H_
#define _IOSQ_H_

#include "sq.h"

class IOSQ;    // forward definition
typedef boost::shared_ptr<IOSQ>        SharedIOSQPtr;
#define CAST_TO_IOSQ(shared_trackable_ptr)  \
        boost::shared_polymorphic_downcast<IOSQ>(shared_trackable_ptr);


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
     */
    IOSQ(int fd);
    virtual ~IOSQ();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedIOSQPtr NullIOSQPtr;

    uint8_t GetPriority() { return mPriority; }

    /**
     * Initialize this object and allocates contiguous Q content memory.
     * @param qId Pass the queue's ID
     * @param numEntries Pass the number of elements within the Q (1 - based)
     * @param cqId Pass the assoc CQ ID to which this SQ will be associated
     * @param priority Pass this Q's priority value, must be a 2 bit value
     */
    void Init(uint16_t qId, uint32_t numEntries, uint16_t cqId,
        uint8_t priority);

    /**
     * Initialize this object and allocates discontiguous Q content memory.
     * @param qId Pass the queue's ID
     * @param numEntries Pass the number of elements within the Q (1 - based)
     * @param memBuffer Hand off this Q's memory. It must satisfy
     *      MemBuffer.GetBufSize()>=(numEntries * gCtrlrConfig->GetIOSQES(). It
     *      must only ever be accessed as RO. Writing to this buffer will have
     *      unpredictable results.
     * @param cqId Pass the assoc CQ ID to which this SQ will be associated
     * @param priority Pass this Q's priority value, must be a 2 bit value
     */
    void Init(uint16_t qId, uint32_t numEntries,
        const SharedMemBufferPtr memBuffer, uint16_t cqId, uint8_t priority);


private:
    IOSQ();

    uint8_t mPriority;
};


#endif
