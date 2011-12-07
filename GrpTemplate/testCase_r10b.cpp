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

#include "testCase_r10b.h"
#include "globals.h"
#include "../Utils/kernelAPI.h"


TestCase_r10b::TestCase_r10b(int fd, string grpName, string testName) :
    Test(fd, grpName, testName, SPECREV_10b)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section ?");
    mTestDesc.SetShort(     "This is a template test");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "This test is a template for future use. This string description has "
        "no length restrictions. See header file for class TestDescribe for "
        "further details.c");
}


TestCase_r10b::~TestCase_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


TestCase_r10b::
TestCase_r10b(const TestCase_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


TestCase_r10b &
TestCase_r10b::operator=(const TestCase_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


bool
TestCase_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) none
     *  \endverbatim
     */

     // Place local variables here

    return true;
}
