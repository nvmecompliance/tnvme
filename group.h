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

#ifndef _GROUP_H_
#define _GROUP_H_

#include <string>
#include <deque>
#include "tnvme.h"
#include "test.h"


/// Use to append a new x.0 test number at the group level
#define APPEND_TEST_AT_GROUP_LEVEL(test, fd, grpName)       \
    {                                                       \
        deque<Test *> tmp;                                  \
        tmp.push_back(new test(fd, #grpName, #test));       \
        mTests.push_back(tmp);                              \
    }

/// Use to append a new x.y test number at the test level
#define APPEND_TEST_AT_TEST_LEVEL(test, fd, grpName)        \
    mTests.back().push_back(new test(fd, #grpName));

/// To allow formatting the group information string
#define FORMAT_GROUP_DESCRIPTION(stdStr, grpObjPtr)                 \
    {                                                               \
        char charray[80];                                           \
        sprintf(charray, "%ld:Group: %s",                           \
                grpObjPtr->GetGroupNumber(),                        \
                grpObjPtr->GetGroupDescription().c_str());          \
        stdStr = charray;                                           \
    }

// To allow formatting a test string identically where ever used
#define FORMAT_TEST_NUM(stdStr, padStr, major, minor)               \
    {                                                               \
        char num[40];                                               \
        sprintf(num, "%s%ld.%ld:Test: ", padStr, major, minor);     \
        stdStr = num;                                               \
    }

typedef size_t TestIteratorType;


/**
* This class is the base/interface class for all groups. This class purposely
* enforces children to conform and inherit certain functionality to handle
* mundane tasks.
*
* @note This class will not throw exceptions.
*/
class Group
{
public:
    /**
     * @param grpNum Pass the assigned group number, globally unique ID
     * @param specRev Pass which compliance is needed to target
     * @param desc Pass a 1-line comment describing group purpose, maximum
     *      number of characters allowed: MAX_CHAR_PER_LINE_DESCRIPTION
     */
    Group(size_t grpNum, SpecRev specRev, string desc);
    virtual ~Group();

    /**
     * @return The externally assigned group number of this instance
     */
    size_t GetGroupNumber() { return mGrpNum; }

    /**
     * Get group information
     * @return Formatted information
     */
    string GetGroupDescription() { return mGrpDesc; }

    /**
     * Get a group summary of all the containing tests
     * @param verbose Pass information verbosity level
     * @return Formatted information
     */
    string GetGroupSummary(bool verbose);

    /**
     * Get formatted test information
     * @param verbose Pass information verbosity level
     * @param tr Pass the test case number to consider
     * @return Formatted information
     */
    string GetTestDescription(bool verbose, TestRef &tr);

    /**
     * Used to allow iterating through all the tests contained within this
     * group. It is useful when all the major.minor test numbers are unknown.
     */
    TestIteratorType GetTestIterator() { return 0; }

    typedef enum {
        TR_SUCCESS,
        TR_FAIL,
        TR_SKIPPING,
        TR_NOTFOUND
    } TestResult;

    /**
     * Run a spec'd test case and report back.
     * @param testIter Pass the test case iterator
     * @param skipTest Pass the complete list of test which should be skipped
     * @return A TestResult
     */
    TestResult RunTest(TestIteratorType &testIter, vector<TestRef> &skipTest);

    /**
     * Run a spec'd test case and report back.
     * @param tr Pass the test case number to execute
     * @param skipTest Pass the complete list of test which should be skipped
     * @return A TestResult
     */
    TestResult RunTest(TestRef &tr, vector<TestRef> &skipTest);


protected:
    size_t      mGrpNum;
    string      mGrpDesc;
    SpecRev     mSpecRev;

    /// array[major][minor];
    /// major test number: are related at the group level; 1.0, 2.0, 3.0
    /// minor test number: are related at the test level; x.1, x.2, x.3
    deque<deque<Test *> > mTests;

    /**
     * Validate whether or not the spec'd test case exists.
     * @param tr Pass the test case number to consider
     * @return true if it exists, otherwise false
     */
    bool TestExists(TestRef tr);

    /**
     * Convert a user supplied iterator into a test reference of the form
     * (group:major.minor).
     * @param testIter Pass the iterator to convert
     * @param tr Returns the converted equivalent if successful
     * @return true upon success, otherwise false. A false return will
     *         indicate that there are no more tests to execute, if and only if,
     *         one were to keep calling this routine while incrementing the
     *         iterator each time, thus iterating all tests.
     */
    bool IteraterToTestRef(TestIteratorType testIter, TestRef &tr);

    /**
     * Deterines if the test under consideration is one of the ones which
     * must be skipped.
     * @param tr Pass in the present test to consider
     * @param skipTest Pass the complete list of tests to skip execution
     * @return true upon success, otherwise false.
     */
    bool SkippingTest(TestRef &tr, vector<TestRef> &skipTest);


private:
    Group() {}
};


#endif
