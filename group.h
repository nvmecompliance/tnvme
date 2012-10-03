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
#include "globals.h"


/// Use to append a new x.0.0 test number at the XLEVEL
#define APPEND_TEST_AT_XLEVEL(test, grpName)                                  \
    {                                                                         \
        deque<Test *> zlevel;                                                 \
        zlevel.push_back(new test(#grpName, #test));                          \
        deque<deque<Test *> > ylevel;                                         \
        ylevel.push_back(zlevel);                                             \
        mTests.push_back(ylevel);                                             \
        ylevel.clear();                                                       \
        zlevel.clear();                                                       \
    }
/// Use to append a new x.y.0 test number at the YLEVEL
#define APPEND_TEST_AT_YLEVEL(test, grpName)                                  \
    {                                                                         \
        deque<Test *> zlevel;                                                 \
        zlevel.push_back(new test(#grpName, #test));                          \
        mTests.back().push_back(zlevel);                                      \
        zlevel.clear();                                                       \
    }

/// Use to append a new x.y.z test number at the ZLEVEL
#define APPEND_TEST_AT_ZLEVEL(test, grpName)                                  \
    mTests.back().back().push_back(new test(#grpName, #test));


/// To allow formatting the group information string
#define FORMAT_GROUP_DESCRIPTION(stdStr, grpObjPtr)                           \
    {                                                                         \
        char charray[80];                                                     \
        snprintf(charray, 80, "%ld: %s: %s",                                  \
                grpObjPtr->GetGroupNumber(),                                  \
                grpObjPtr->GetClassName().c_str(),                            \
                grpObjPtr->GetGroupDescription().c_str());                    \
        stdStr = charray;                                                     \
    }

// To allow formatting a test string identically where ever used
#define FORMAT_TEST_NUM(stdStr, padStr, x, y, z)                              \
    {                                                                         \
        char num[40];                                                         \
        snprintf(num, 40, "%s%ld.%ld.%ld: Test: ", padStr, x, y, z);          \
        stdStr = num;                                                         \
    }

typedef size_t TestIteratorType;
typedef vector<TestRef> TestSetType;


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
     * @param grpName Pass the name assigned to this group
     * @param desc Pass a 1-line comment describing group purpose, maximum
     *      number of characters allowed: MAX_CHAR_PER_LINE_DESCRIPTION
     */
    Group(size_t grpNum, string grpName, string desc);
    virtual ~Group();

    /**
     * Get the C++ object name assigned to this object. This can be used to
     * locate the source code of a failed test case.
     * @return The C++ assigned name of this test object
     */
    string GetClassName() { return mGrpName; }

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
     * Returns a set of tests which must be run in order to satisfy any test
     * dependencies of the targeted test case.
     * @param target Pass the target test case to execute; could be a
     *        reference to entire an group or a single test case
     * @param dependencies Returns an order set of tests to execute
     * @param tstIdx Returns an index to the 1st test within dependencies
     * @return true upon success, otherwise false
     */
    bool GetTestSet(TestRef &target, TestSetType &dependencies,
        int64_t &tstIdx);

    typedef enum {
        TR_SUCCESS,
        TR_FAIL,
        TR_SKIPPING,
        TR_NOTFOUND
    } TestResult;

    /**
     * Run a spec'd test case within the provided dependencies using tstIdx
     * which references the test to execute. Upon exit the tstIdx will
     * point to the next test within the provided dependencies for future
     * execution. This method should called within a loop until an undesired
     * TestResult is returned or the returned (tstIdx == -1), thus indicating
     * there are no more tests to execute within the dependencies.
     * @note The dependencies & tstIdx should be provided by GetTestSet()
     * @param dependencies Pass the ordered list of tests which are desired to
     *        be executed in sequence, one-at-a time. This set should not
     *        be modified between consecutive calls to this method.
     * @param tstIdx Pass the index which points to the next test to
     *        execute, returns the next test in the dependencies to execute upon
     *        a subsequent call to RunTest(), returns -1 when the last test
     *        within the dependencies has been executed.
     * @param skipTest Pass the complete list of test which should be skipped
     * @param numSkipped Returns the number of tests which were skipped as a
     *        result of a failed test or a test reporting it cannot be executed.
     * @param preserve Pass true if the DUT must be preserve, false if it can be
     *        permanently changed. See cmd line option --preserve
     * @return The result from executing a single test case.
     */
    TestResult RunTest(TestSetType &dependencies, int64_t &tstIdx,
        vector<TestRef> &skipTest, int64_t &numSkipped, bool preserve,
        vector<TestRef> &failedTests, vector<TestRef> &skippedTests);


protected:
    size_t  mGrpNum;
    string  mGrpName;
    string  mGrpDesc;

    /// array[xLevel][yLevel][zLevel];
    /// Refer to: https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering
    deque<deque<deque<Test *> > > mTests;

    /**
     * In coordination with cmd line option --restore, these functions should
     * be over ridden by children to support the saving and restoring of a DUT's
     * configuration, if and only if the tests contained within a group
     * could/would or will modify the permanent DUT configuration. Permanent
     * changes are those defined to carry through a cold hard reset.
     * @return true when the configuration was correctly saved/restored,
     *         otherwise false.
     */
    virtual bool SaveState() { return true; }
    virtual bool RestoreState() {return true; }


private:
    Group() {}
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
     * Convert a user supplied test reference into a iterator.
     * @param tr Pass the test reference to convert
     * @param testIter Returns the converted equivalent if successful
     * @return true upon success, otherwise false.
     */
    bool TestRefToIterator(TestRef tr, TestIteratorType &testIter);

    /**
     * Determines if the test under consideration is one of the ones which
     * must be skipped.
     * @param tr Pass in the present test to consider
     * @param skipTest Pass the complete list of tests to skip execution
     * @return true upon success, otherwise false.
     */
    bool SkippingTest(TestRef &tr, vector<TestRef> &skipTest);

    /**
     * Advance tstIdx so that the tests dependent upon the test case referenced
     * by dependencies[tstIdx] are skipped, the new returned value of tstIdx
     * will point to the next test to execute without dependencies upon the
     * one being reference when the method is called.
     * @param dependencies Pass the ordered list of tests which are desired to
     *        be executed in sequence.
     * @param tstIdx Pass the index which points to the test which all other
     *        tests might depend, returns -1 when there is no next test within
     *        dependencies to execute.
     * @param failed Pass whether or not the dependencies[tstIdx] failed
     *        and is the reason for advancing past dependencies
     * @ returns Returns the number of tests dependent upon the tstIdx iterator.
     */
    int64_t  AdvanceDependencies(TestSetType &dependencies, int64_t &tstIdx,
        bool failed, vector<TestRef> &skippedTests);

};


#endif
