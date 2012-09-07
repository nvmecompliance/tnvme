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

#ifndef _IOCQ_H_
#define _IOCQ_H_

#include "cq.h"

class IOCQ;    // forward definition
typedef boost::shared_ptr<IOCQ>             SharedIOCQPtr;
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
     * @param numEntries Pass the number of elements within the Q (1 - based)
     * @param irqEnabled Pass true if IRQ's are to be enabled for this Q
     * @param irqVec If (irqEnabled==true) then spec's the IRQ's vector. Value
     *        must be among the set from (0 - (n-1)) where n is is spec'd
     *        during a call to CtrlrConfig::SetIrqScheme().
     */
    void Init(uint16_t qId, uint32_t numEntries, bool irqEnabled,
        uint16_t irqVec);

    /**
     * Initialize this object and allocates discontiguous Q content memory.
     * @param qId Pass the queue's ID
     * @param numEntries Pass the number of elements within the Q (1 - based)
     * @param memBuffer Hand off this Q's memory. It must satisfy
     *      MemBuffer.GetBufSize()>=(numEntries * gCtrlrConfig->GetIOCQES(). It
     *      must only ever be accessed as RO. Writing to this buffer will have
     *      unpredictable results.
     * @param irqEnabled Pass true if IRQ's are to be enabled for this Q
     * @param irqVec If (irqEnabled==true) then spec's the IRQ's vector. Value
     *        must be among the set from (0 - (n-1)) where n is is spec'd
     *        during a call to CtrlrConfig::SetIrqScheme()
     */
    void Init(uint16_t qId, uint32_t numEntries,
        const SharedMemBufferPtr memBuffer, bool irqEnabled, uint16_t irqVec);


private:
    IOCQ();
};


#endif
