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

#ifndef _FIDIRQVEC_r11_H_
#define _FIDIRQVEC_r11_H_

#include "fidIRQVec_r10b.h"
#include "../Queues/acq.h"
#include "../Queues/asq.h"
#include "../Queues/iocq.h"

namespace GrpAdminSetGetFeatCombo {


/** \verbatim
 * -----------------------------------------------------------------------------
 * ----------------Mandatory rules for children to follow-----------------------
 * -----------------------------------------------------------------------------
 * 1) See notes in the header file of the Test base class
 * \endverbatim
 */
class FIDIRQVec_r11 : public FIDIRQVec_r10b
{
public:
    FIDIRQVec_r11(string grpName, string testName);
    virtual ~FIDIRQVec_r11();

    /**
     * IMPORTANT: Read Test::Clone() header comment.
     */
    virtual FIDIRQVec_r11 *Clone() const
        { return new FIDIRQVec_r11(*this); }
    FIDIRQVec_r11 &operator=(const FIDIRQVec_r11 &other);
    FIDIRQVec_r11(const FIDIRQVec_r11 &other);


protected:
    virtual void RunCoreTest();
    virtual RunType RunnableCoreTest(bool preserve);

private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator=().
    ///////////////////////////////////////////////////////////////////////////
    void reset(SharedIOCQPtr iocq, SharedASQPtr asq, SharedACQPtr acq);
    SharedIOCQPtr createQueues(SharedASQPtr asq, SharedACQPtr acq,
        uint16_t ivec);

    enum nvme_irq_type scheme;
    uint16_t numIrqSupport;
};

}   // namespace

#endif
