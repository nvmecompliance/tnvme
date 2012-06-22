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

#ifndef _MANYSQTOCQASSOC_R10B_H_
#define _MANYSQTOCQASSOC_R10B_H_

#include <map>

#include "test.h"
#include "globals.h"
#include "../Cmds/identify.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Cmds/write.h"
#include "../Utils/queues.h"

namespace GrpQueues {


/** \verbatim
 * -----------------------------------------------------------------------------
 * ----------------Mandatory rules for children to follow-----------------------
 * -----------------------------------------------------------------------------
 * 1) See notes in the header file of the Test base class
 * \endverbatim
 */
class ManySQtoCQAssoc_r10b : public Test
{
public:
    ManySQtoCQAssoc_r10b(string grpName, string testName);
    virtual ~ManySQtoCQAssoc_r10b();

    /**
     * IMPORTANT: Read Test::Clone() header comment.
     */
    virtual ManySQtoCQAssoc_r10b *Clone() const
        { return new ManySQtoCQAssoc_r10b(*this); }
    ManySQtoCQAssoc_r10b &operator=(const ManySQtoCQAssoc_r10b &other);
    ManySQtoCQAssoc_r10b(const ManySQtoCQAssoc_r10b &other);


protected:
    virtual void RunCoreTest();
    virtual RunType RunnableCoreTest(bool preserve);;


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator=().
    ///////////////////////////////////////////////////////////////////////////
    SharedWritePtr SetWriteCmd();
    void ReapIOCQAndVerifyCE(SharedIOCQPtr iocq, uint32_t numTil,
        vector<uint32_t> mSQIDToSQHDVector);
};

}   // namespace

#endif
