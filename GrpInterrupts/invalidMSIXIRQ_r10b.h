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

#ifndef _INVALIDMSIXIRQ_r10b_H_
#define _INVALIDMSIXIRQ_r10b_H_

#include "test.h"
#include "../Queues/acq.h"
#include "../Queues/asq.h"

namespace GrpInterrupts {


/** \verbatim
 * -----------------------------------------------------------------------------
 * ----------------Mandatory rules for children to follow-----------------------
 * -----------------------------------------------------------------------------
 * 1) See notes in the header file of the Test base class
 * \endverbatim
 */
class InvalidMSIXIRQ_r10b : public Test
{
public:
    InvalidMSIXIRQ_r10b(string grpName, string testName);
    virtual ~InvalidMSIXIRQ_r10b();

    /**
     * IMPORTANT: Read Test::Clone() header comment.
     */
    virtual InvalidMSIXIRQ_r10b *Clone() const
        { return new InvalidMSIXIRQ_r10b(*this); }
    InvalidMSIXIRQ_r10b &operator=(const InvalidMSIXIRQ_r10b &other);
    InvalidMSIXIRQ_r10b(const InvalidMSIXIRQ_r10b &other);


protected:
    virtual void RunCoreTest();
    virtual RunType RunnableCoreTest(bool preserve);


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator=().
    ///////////////////////////////////////////////////////////////////////////
    void ASQCmdToxify(SharedASQPtr asq, uint16_t illegalIrqVec);
    void SendToxicCmd(SharedASQPtr asq, SharedACQPtr acq,
        SharedCmdPtr cmd, uint16_t illegalIrqVec);
};

}   // namespace

#endif
