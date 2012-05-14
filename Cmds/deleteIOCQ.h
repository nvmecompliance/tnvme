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

#ifndef _DELETEIOCQ_H_
#define _DELETEIOCQ_H_

#include "cmd.h"
#include "../Queues/iocq.h"


class DeleteIOCQ;    // forward definition
typedef boost::shared_ptr<DeleteIOCQ>             SharedDeleteIOCQPtr;
typedef boost::shared_ptr<const DeleteIOCQ>       ConstSharedDeleteIOCQPtr;
#define CAST_TO_DELETEIOCQ(shared_trackable_ptr)  \
        boost::shared_polymorphic_downcast<DeleteIOCQ>(shared_trackable_ptr);


/**
* This class implements the Delete IO Completion Queue admin cmd. After
* instantiation the Init() methods must be called to attain something useful.
*
* @note This class may throw exceptions.
*/
class DeleteIOCQ : public Cmd
{
public:
    DeleteIOCQ();
    virtual ~DeleteIOCQ();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedDeleteIOCQPtr NullDeleteIOCQPtr;
    static const uint8_t Opcode;

    /**
     * Initialize this object and prepares it to send to the hdw.
     * @param iocq Pass the IOCQ object which will initialize this cmd.
     */
    void Init(const SharedIOCQPtr iocq);
};


#endif
