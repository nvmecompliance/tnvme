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

#ifndef _STARTINGLBABARE_r10b_H_
#define _STARTINGLBABARE_r10b_H_

#include "test.h"
#include "../Cmds/read.h"
#include "../Cmds/write.h"
#include "../Cmds/identify.h"

namespace GrpNVMWriteReadCombo {


/** \verbatim
 * -----------------------------------------------------------------------------
 * ----------------Mandatory rules for children to follow-----------------------
 * -----------------------------------------------------------------------------
 * 1) See notes in the header file of the Test base class
 * \endverbatim
 */
class StartingLBABare_r10b : public Test
{
public:
    StartingLBABare_r10b(string grpName, string testName);
    virtual ~StartingLBABare_r10b();

    /**
     * IMPORTANT: Read Test::Clone() header comment.
     */
    virtual StartingLBABare_r10b *Clone() const
        { return new StartingLBABare_r10b(*this); }
    StartingLBABare_r10b &operator=(const StartingLBABare_r10b &other);
    StartingLBABare_r10b(const StartingLBABare_r10b &other);


protected:
    virtual void RunCoreTest();
    virtual RunType RunnableCoreTest(bool preserve);


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator=().
    ///////////////////////////////////////////////////////////////////////////
    void VerifyDataPat(SharedReadPtr readCmd, SharedMemBufferPtr wrPayload);
    void ResizeDataBuf(SharedReadPtr &readCmd, SharedWritePtr &writeCmd,
        ConstSharedIdentifyPtr namSpcPtr, uint64_t maxWrBlks,
        send_64b_bitmask prpBitmask);    
};

}   // namespace

#endif
