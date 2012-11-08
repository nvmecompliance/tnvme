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

#ifndef _ASYNCEVENTREQ_H_
#define _ASYNCEVENTREQ_H_

#include "cmd.h"


class AsyncEventReq;    // forward definition
typedef boost::shared_ptr<AsyncEventReq>        SharedAsyncEventReqPtr;
typedef boost::shared_ptr<const AsyncEventReq>
    ConstSharedAsyncEventRequestPtr;
#define CAST_TO_ASYNCEVENT(shared_trackable_ptr)   \
    boost::shared_polymorphic_downcast<AsyncEventRequest>(shared_trackable_ptr);


/**
* This class implements the AsyncEventReq admin cmd
*
* @note This class may throw exceptions.
*/
class AsyncEventReq : public Cmd
{
public:
    AsyncEventReq();
    virtual ~AsyncEventReq();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedAsyncEventReqPtr NullAsyncEventReqPtr;
    static const uint8_t Opcode;

};


#endif
