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

#include "tnvme.h"
#include "group.h"
#include "globals.h"

#define PAD_INDENT_LVL1         "    "
#define PAD_INDENT_LVL2         "      "
#define NEWLINE                 "\n"


Group::Group(size_t grpNum, SpecRev specRev, string desc)
{
    mGrpNum = grpNum;
    mSpecRev = specRev;

    if (desc.length() > MAX_CHAR_PER_LINE_DESCRIPTION) {
        LOG_ERR("Group description length violation, concatenating \"%s\"",
            desc.c_str());
    }
    mGrpDesc = desc.substr(0, MAX_CHAR_PER_LINE_DESCRIPTION);
}


Group::~Group()
{
    while (mTests.size()) {
        while (mTests.back().size()) {
            delete (Test *)(mTests.back().back());
            mTests.back().pop_back();
        }
        mTests.pop_back();
    }
}


string
Group::GetGroupSummary(bool verbose)
{
    string work;
    TestRef tr;

    tr.group = mGrpNum;
    for (tr.major = 0; tr.major < mTests.size(); tr.major++) {
        for (tr.minor = 0; tr.minor < mTests[tr.major].size(); tr.minor++) {
            work += GetTestDescription(verbose, tr);
        }
    }

    return work;
}


string
Group::GetTestDescription(bool verbose, TestRef &tr)
{
    string work;

    FORMAT_TEST_NUM(work, PAD_INDENT_LVL1, tr.major, tr.minor)

    if (TestExists(tr) == false) {
        work += "unknown test";
        work += NEWLINE;

    } else {
        if (verbose) {
            work += mTests[tr.major][tr.minor]->GetShortDescription();
            work += NEWLINE;
            work += PAD_INDENT_LVL2;
            work += "Compliance: ";
            work += mTests[tr.major][tr.minor]->GetComplianceDescription();
            work += NEWLINE;
            work += mTests[tr.major][tr.minor]->
                GetLongDescription(true, sizeof(PAD_INDENT_LVL2) + 2);
        } else {
            work += mTests[tr.major][tr.minor]->GetShortDescription();
            work += NEWLINE;
        }
    }

    return work;
}


bool
Group::TestExists(TestRef tr)
{
    if ((tr.group != mGrpNum) ||
        (tr.major >= mTests.size()) ||
        (tr.minor >= mTests[tr.major].size())) {
        // Uncomment to track down iterator issues, otherwise it ends up being
        // a nuisance in the logs, cluttering the readability.
        //LOG_DBG("Test case %ld:%ld.%ld does not exist within group %ld",
        //    tr.group, tr.major, tr.minor, mGrpNum);
        return false;
    }
    return true;
}


bool
Group::IteraterToTestRef(TestIteratorType testIter, TestRef &tr)
{
    size_t count = 0;
    size_t major = 0;
    size_t minor = 0;

    // This loop is attempting to take the testIter, it should be pointing to
    // the next test to consider for execution. However that doesn't mean there
    // actually is a test object at the testIter index within mTests[][]. Start
    // from the beginning and traverse the entire matrix to attain a valid test
    // object for execution.
    LOG_DBG("Traverse test matrix %ld seeking test @ iterator idx=%ld",
        mGrpNum, testIter);
    while (count < testIter) {
        if (TestExists(TestRef(mGrpNum, major, minor+1))) {
            minor++;        // same group level, but the next test level count
            count++;
        } else if (TestExists(TestRef(mGrpNum, major+1, 0))) {
            major++;
            minor = 0;      // new group level, restart test level counting
            count++;
        } else {            // no next test, never found it
            break;
        }
    }

    if (count != testIter)
        return false;

    tr.group = mGrpNum;
    tr.major = major;
    tr.minor = minor;
    return true;
}


Group::TestResult
Group::RunTest(TestIteratorType &testIter, vector<TestRef> &skipTest)
{
    TestRef tr;

    if (IteraterToTestRef(testIter, tr) == false)
        return TR_NOTFOUND;

    testIter++;     // next test to consider for execution in the future
    return RunTest(tr, skipTest);
}


Group::TestResult
Group::RunTest(TestRef &tr, vector<TestRef> &skipTest)
{
    string work;

    // Take out a valid deque<>::iterator to the test under execution
    if (TestExists(tr) == false)
        return TR_NOTFOUND;
    deque<Test *>::iterator testCase = mTests[tr.major].begin();
    advance(testCase, tr.minor);

    LOG_NRM("-----------------START TEST-----------------");
    FORMAT_GROUP_DESCRIPTION(work, this)
    LOG_NRM("%s", work.c_str());
    FORMAT_TEST_NUM(work, "", tr.major, tr.minor)
    work += (*testCase)->GetShortDescription();
    LOG_NRM("%s", work.c_str());
    LOG_NRM("Compliance: %s", (*testCase)->GetComplianceDescription().c_str());
    LOG_NRM("%s", (*testCase)->GetLongDescription(false, 0).c_str());

    TestResult result;
    if (SkippingTest(tr, skipTest))
        result = TR_SKIPPING;
    else
        result = (*testCase)->Run() ? TR_SUCCESS: TR_FAIL;
    LOG_NRM("------------------END TEST------------------");

    // Guarantee nothing residual or unintended is left around. Enforce this
    // by destroying the existing test obj and replace it with a clone of
    // itself so looping tests over can still be supported.
    LOG_DBG("Enforcing test obj cleanup, cloning & destroying");
    Test *cleanMeUp = (*testCase);  // Refer to test obj
    deque<Test *>::iterator insertPos = mTests[tr.major].erase(testCase);
    mTests[tr.major].insert(insertPos, cleanMeUp->Clone()); // cloning test obj
    delete cleanMeUp;               // Destructing test obj

    return result;
}


bool
Group::SkippingTest(TestRef &tr, vector<TestRef> &skipTest)
{
    for (size_t i = 0; i < skipTest.size(); i++) {
         if ((tr.group == skipTest[i].group) &&
             (tr.major == skipTest[i].major) &&
             (tr.minor == skipTest[i].minor)) {

            LOG_NRM("Instructed to skip specific test: %ld:%ld.%ld",
                tr.group, tr.major, tr.minor);
            return true;
         } else if ((tr.group == skipTest[i].group) &&
             (UINT_MAX == skipTest[i].major) &&
             (UINT_MAX == skipTest[i].minor)) {

            LOG_NRM("Instructed to skip entire group: %ld (test:%ld.%ld)",
                tr.group, tr.major, tr.minor);
            return true;
         }
    }
    return false;
}
