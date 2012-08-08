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

#ifndef _ASQ_H_
#define _ASQ_H_

#include "sq.h"

class ASQ;    // forward definition
typedef boost::shared_ptr<ASQ>        SharedASQPtr;
#define CAST_TO_ASQ(shared_trackable_ptr)  \
        boost::shared_polymorphic_downcast<ASQ>(shared_trackable_ptr);


/**
* This class is meant to be instantiated and represents an ASQ. After
* instantiation the Init() methods must be called to attain something useful.
*
* @note This class may throw exceptions.
*/
class ASQ : public SQ
{
public:
    /**
     * @param fd Pass the opened file descriptor for the device under test
     */
    ASQ(int fd);
    virtual ~ASQ();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedASQPtr NullASQPtr;
    static const uint16_t IDEAL_ELEMENT_SIZE;

    /**
     * Initialize this object and allocates a contiguous ACQ
     * @param numEntries Pass the number of elements within the Q (1 - based)
     */
    void Init(uint32_t numEntries);

    /**
     * Issue the specified cmd to this queue, but does not ring any doorbell.
     * @param cmd Pass the cmd to send to this queue.
     * @param uniqueId Returns the dnvme assigned unique cmd ID
     */
    virtual void Send(SharedCmdPtr cmd, uint16_t &uniqueId);


private:
    ASQ();
};


#endif
