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

#ifndef _ACQ_H_
#define _ACQ_H_

#include "cq.h"

class ACQ;    // forward definition
typedef boost::shared_ptr<ACQ>              SharedACQPtr;
#define CAST_TO_ACQ(shared_trackable_ptr)   \
        boost::shared_polymorphic_downcast<ACQ>(shared_trackable_ptr);


/**
* This class is meant to be instantiated and represents an ACQ. After
* instantiation the Init() methods must be called to attain something useful.
*
* @note This class may throw exceptions.
*/
class ACQ : public CQ
{
public:
    /**
     * @param fd Pass the opened file descriptor for the device under test
     */
    ACQ(int fd);
    virtual ~ACQ();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedACQPtr NullACQPtr;
    static const uint16_t IDEAL_ELEMENT_SIZE;

    /**
     * Initialize this object and allocates a contiguous ACQ
     * @param numEntries Pass the number of elements within the Q (1 - based)
     */
    void Init(uint32_t numEntries);


private:
    ACQ();
};


#endif
