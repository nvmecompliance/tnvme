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

#ifndef _DELETEIOSQ_H_
#define _DELETEIOSQ_H_

#include "cmd.h"
#include "../Queues/iosq.h"


class DeleteIOSQ;    // forward definition
typedef boost::shared_ptr<DeleteIOSQ>             SharedDeleteIOSQPtr;
typedef boost::shared_ptr<const DeleteIOSQ>       ConstSharedDeleteIOSQPtr;
#define CAST_TO_DELETEIOSQ(shared_trackable_ptr)  \
        boost::shared_polymorphic_downcast<DeleteIOSQ>(shared_trackable_ptr);


/**
* This class implements the Delete IO Submission Queue admin cmd. After
* instantiation the Init() methods must be called to attain something useful.
*
* @note This class may throw exceptions.
*/
class DeleteIOSQ : public Cmd
{
public:
    DeleteIOSQ();
    virtual ~DeleteIOSQ();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedDeleteIOSQPtr NullDeleteIOSQPtr;
    static const uint8_t Opcode;

    /**
     * Initialize this object and prepares it to send to the hdw.
     * @param iosq Pass the IOCQ object which will initialize this cmd.
     */
    void Init(const SharedIOSQPtr iosq);
};


#endif
