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

#ifndef _MAXQSIZEEXCEED_r10b_H_
#define _MAXQSIZEEXCEED_r10b_H_

#include <list>
#include "test.h"

namespace GrpAdminCreateIOSQCmd {


/** \verbatim
 * -----------------------------------------------------------------------------
 * ----------------Mandatory rules for children to follow-----------------------
 * -----------------------------------------------------------------------------
 * 1) See notes in the header file of the Test base class
 * \endverbatim
 */
class MaxQSizeExceed_r10b : public Test
{
public:
    MaxQSizeExceed_r10b(string grpName, string testName);
    virtual ~MaxQSizeExceed_r10b();

    /**
     * IMPORTANT: Read Test::Clone() header comment.
     */
    virtual MaxQSizeExceed_r10b *Clone() const
        { return new MaxQSizeExceed_r10b(*this); }
    MaxQSizeExceed_r10b &operator=(const MaxQSizeExceed_r10b &other);
    MaxQSizeExceed_r10b(const MaxQSizeExceed_r10b &other);


protected:
    virtual void RunCoreTest();
    virtual RunType RunnableCoreTest(bool preserve);


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator=().
    ///////////////////////////////////////////////////////////////////////////
    list<uint32_t> GetIllegalQSizes(uint32_t maxIOQEntries);
};

}   // namespace

#endif
