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

#include "grpTemplate.h"
#include "testCase_r10b.h"

namespace GrpTemplate {


GrpTemplate::GrpTemplate(size_t grpNum, SpecRev specRev, ErrorRegs errRegs,
    int fd) :
    Group(grpNum, specRev, "Template Group")
{
    // IMPORTANT: Once a test case is assigned a position in the vector, i.e.
    //            a test index/number of the form major.minor, then it should
    //            reside in that position forever so test reference numbers
    //            don't change per release. Future tests can be appended at
    //            either the group level or the test level.
    //            Tests 0.0, 1.0, <next_test_num=2>.0  Major num; group level
    //            Tests x.0, x.1, x.<next_test_num=2>  Minor num; test level
    switch (mSpecRev) {
    case SPECREV_10b:
        APPEND_TEST_AT_GROUP_LEVEL(TestCase_r10b, fd, GrpTemplate, errRegs)
        break;

    default:
    case SPECREVTYPE_FENCE:
        LOG_DBG("Object created with an unknown SpecRev=%d", specRev);
        break;
    }
}


GrpTemplate::~GrpTemplate()
{
    // mTests deallocated in parent
}

}   // namespace
