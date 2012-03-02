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


/// Use to append a new x.0.0 test number at the XLEVEL
#define APPEND_TEST_AT_XLEVEL(test, fd, grpName, errRegs)                     \
    {                                                                         \
        deque<Test *> zlevel;                                                 \
        zlevel.push_back(new test(fd, #grpName, #test, errRegs));             \
        deque<deque<Test *> > ylevel;                                         \
        ylevel.push_back(zlevel);                                             \
        mTests.push_back(ylevel);                                             \
        ylevel.clear();                                                       \
        zlevel.clear();                                                       \
    }
/// Use to append a new x.y.0 test number at the YLEVEL
#define APPEND_TEST_AT_YLEVEL(test, fd, grpName, errRegs)                     \
    {                                                                         \
        deque<Test *> zlevel;                                                 \
        zlevel.push_back(new test(fd, #grpName, #test, errRegs));             \
        mTests.back().push_back(zlevel);                                      \
        zlevel.clear();                                                       \
    }

/// Use to append a new x.y.z test number at the ZLEVEL
#define APPEND_TEST_AT_ZLEVEL(test, fd, grpName, errRegs)                     \
    mTests.back().back().push_back(new test(fd, #grpName, #test, errRegs));


/// To allow formatting the group information string
#define FORMAT_GROUP_DESCRIPTION(stdStr, grpObjPtr)                           \
    {                                                                         \
        char charray[80];                                                     \
        sprintf(charray, "%ld: Group:%s",                                     \
                grpObjPtr->GetGroupNumber(),                                  \
                grpObjPtr->GetGroupDescription().c_str());                    \
        stdStr = charray;                                                     \
    }

// To allow formatting a test string identically where ever used
#define FORMAT_TEST_NUM(stdStr, padStr, x, y, z)                              \
    {                                                                         \
        char num[40];                                                         \
        sprintf(num, "%s%ld.%ld.%ld: Test:", padStr, x, y, z);                \
        stdStr = num;                                                         \
    }

typedef size_t TestIteratorType;


/**
* This class is the base/interface class for all groups. This class purposely
* enforces children to conform and inherit functionality to handle mundane tasks
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

    /// Used to allow iterating through all the tests contained within a group.
    TestIteratorType GetTestIterator() { return 0; }

    /**
     * Convert a user supplied test reference into a iterator.
     * @param tr Pass the test reference to convert
     * @param testIter Returns the converted equivalent if successful
     * @return true upon success, otherwise false.
     */
    bool TestRefToIterator(TestRef tr, TestIteratorType &testIter);

    typedef enum {
        TR_SUCCESS,
        TR_FAIL,
        TR_SKIPPING,
        TR_NOTFOUND
    } TestResult;

    /**
     * Run a spec'd test case and report back. The iterator is incremented to
     * the next possible test when returning, thus it will allow to iterate over
     * all tests contained within this group if this method is called repeatedly
     * @param testIter Pass the test case iterator
     * @param skipTest Pass the complete list of test which should be skipped
     * @return A TestResult
     */
    TestResult RunTest(TestIteratorType &testIter, vector<TestRef> &skipTest);

    /**
     * Run a spec'd test case and report back. This method does NOT allow
     * iterating over tests wihtin this group.
     * @param tr Pass the test case number to execute
     * @param skipTest Pass the complete list of test which should be skipped
     * @return A TestResult
     */
    TestResult RunTest(TestRef &tr, vector<TestRef> &skipTest);

    typedef enum {
        TD_ZERO,            // zero dependency (xLevel)
        TD_CONFIG,          // configuration dependency (yLevel)
        TD_SEQUENCE,        // sequence dependency (zLevel)

        TD_FENCE            // always must be last element
    } TestDepends;

    /**
     * Get the dependency requirements for an individual test. For full details:
     * https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering.
     * *
     *      retVal          seqDepend                    cfgDepend
     *      --------------------------------------------------------------------
     *      TD_FENCE       ignore(==-1)         ignore(cfgDepend == test)
     *      TD_ZERO        ignore(==-1)         ignore(cfgDepend == test)
     *      TD_CONFIG      ignore(==-1)          valid(cfgDepend != test)
     *      TD_SEQUENCE     valid(!=-1)          valid(cfgDepend != test)
     * @param test Pass test reference to decipher its dependency requirements
     * @param cfgDepend Returns the corresponding configuration dependency;
     *        (cfgDepend == test) when it should be ignored
     * @param seqDepend Returns the corresponding sequence dependency;
     *        (seqDepend == -1) when it should be ignored
     * @return true upon success, otherwise failed to locate or decode something
t
     */
    bool GetTestDependency(TestRef test, TestRef &cfgDepend,
        TestIteratorType &seqDepend);


protected:
    size_t      mGrpNum;
    string      mGrpDesc;
    SpecRev     mSpecRev;

    /// array[xLevel][yLevel][zLevel];
    /// Refer to: https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering
    deque<deque<deque<Test *> > > mTests;

    /**
     * Validate whether or not the spec'd test case exists.
     * @param tr Pass the test case number to consider
     * @return true if it exists, otherwise false
     */
    bool TestExists(TestRef tr);

    /**
     * Convert a user supplied iterator into a test reference.
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
