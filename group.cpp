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


Group::Group(size_t grpNum, SpecRev specRev, string grpName, string desc)
{
    mGrpNum = grpNum;
    mGrpName = grpName;
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
            while (mTests.back().back().size()) {
                delete (Test *)(mTests.back().back().back());
                mTests.back().back().pop_back();
            }
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
    for (tr.xLev = 0; tr.xLev < mTests.size(); tr.xLev++) {
        for (tr.yLev = 0; tr.yLev < mTests[tr.xLev].size(); tr.yLev++) {
            for (tr.zLev = 0; tr.zLev < mTests[tr.xLev][tr.yLev].size();
                tr.zLev++) {

                work += GetTestDescription(verbose, tr);
            }
        }
    }

    return work;
}


string
Group::GetTestDescription(bool verbose, TestRef &tr)
{
    string work;

    FORMAT_TEST_NUM(work, PAD_INDENT_LVL1, tr.xLev, tr.yLev, tr.zLev)

    if (TestExists(tr) == false) {
        work += "unknown test";
        work += NEWLINE;

    } else if (verbose) {
        work += mTests[tr.xLev][tr.yLev][tr.zLev]->GetShortDescription();
        work += NEWLINE;
        work += PAD_INDENT_LVL2;
        work += "Compliance: ";
        work += mTests[tr.xLev][tr.yLev][tr.zLev]->GetComplianceDescription();
        work += NEWLINE;
        work += mTests[tr.xLev][tr.yLev][tr.zLev]->
            GetLongDescription(true, sizeof(PAD_INDENT_LVL2) + 2);
    } else {
        work += mTests[tr.xLev][tr.yLev][tr.zLev]->GetShortDescription();
        work += NEWLINE;
    }

    return work;
}


bool
Group::TestExists(TestRef tr)
{
    if ((tr.group != mGrpNum) ||
        (tr.xLev >= mTests.size()) ||
        (tr.yLev >= mTests[tr.xLev].size()) ||
        (tr.zLev >= mTests[tr.xLev][tr.yLev].size())) {
        LOG_DBG("Test case %ld:%ld.%ld.%ld does not exist within group",
            tr.group, tr.xLev, tr.yLev, tr.zLev);
        return false;
    }
    return true;
}


bool
Group::IteraterToTestRef(TestIteratorType testIter, TestRef &tr)
{
    size_t count = 0;
    size_t x = 0, y = 0, z = 0;

    // This loop is attempting to take the testIter, it should be pointing to
    // the next test to consider for execution. However that doesn't mean there
    // actually is a test object at the testIter index within mTests[][][].
    // Start from the beginning and traverse the entire matrix to attain a
    // valid test object for execution.
    LOG_DBG("Parse mTest matrix %ld seeking test @ iter=%ld",
        mGrpNum, testIter);
    while (count < testIter) {
        if (TestExists(TestRef(mGrpNum, x, y, z+1))) {
            z++;
            count++;
        } else if (TestExists(TestRef(mGrpNum, x, y+1, 0))) {
            y++;
            z = 0;
            count++;
        } else if (TestExists(TestRef(mGrpNum, x+1, 0, 0))) {
            x++;
            y = 0;
            z = 0;
            count++;
        } else {            // no next test, never found it
            break;
        }
    }

    if (count != testIter)
        return false;

    tr.group = mGrpNum;
    tr.xLev = x;
    tr.yLev = y;
    tr.zLev = z;
    return true;
}


bool
Group::TestRefToIterator(TestRef tr, TestIteratorType &testIter)
{
    TestRef proposedTr;
    testIter = GetTestIterator();

    LOG_DBG("Parse mTest matrix %ld seeking test @ %ld:%ld.%ld",
        mGrpNum, tr.xLev, tr.yLev, tr.zLev);
    while (IteraterToTestRef(testIter, proposedTr)) {
        if (tr == proposedTr)
            return true;
        testIter++;
    }

    LOG_ERR("Unable to locate targeted test");
    return false;
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

    if (TestExists(tr) == false)
        return TR_NOTFOUND;

    // Take out a valid deque<>::iterator to the test under execution
    deque<Test *>::iterator myTest = mTests[tr.xLev][tr.yLev].begin();
    advance(myTest, tr.zLev);

    LOG_NRM("-----------------START TEST-----------------");
    FORMAT_GROUP_DESCRIPTION(work, this)
    LOG_NRM("%s", work.c_str());
    FORMAT_TEST_NUM(work, "", tr.xLev, tr.yLev, tr.zLev)
    work += (*myTest)->GetShortDescription();
    LOG_NRM("%s", work.c_str());
    LOG_NRM("Compliance: %s", (*myTest)->GetComplianceDescription().c_str());
    LOG_NRM("%s", (*myTest)->GetLongDescription(false, 0).c_str());

    TestResult result;
    if (SkippingTest(tr, skipTest)) {
        result = TR_SKIPPING;
    } else {
        result = (*myTest)->Run() ? TR_SUCCESS: TR_FAIL;
        if (result == TR_FAIL) {
            FORMAT_GROUP_DESCRIPTION(work, this)
            LOG_NRM("  %s", work.c_str());
            FORMAT_TEST_NUM(work, "", tr.xLev, tr.yLev, tr.zLev)
            work += (*myTest)->GetShortDescription();
            LOG_NRM("  %s", work.c_str());
            LOG_NRM("  Compliance: %s", (*myTest)->GetComplianceDescription().c_str());
            LOG_NRM("  %s", (*myTest)->GetLongDescription(false, 0).c_str());
        }
    }
    LOG_NRM("------------------END TEST------------------");

    // Guarantee nothing residual or unintended is left around. Enforce this
    // by destroying the existing test obj and replace it with a clone of
    // itself so looping tests over can still be supported.
    LOG_DBG("Enforcing test obj cleanup, cloning & destroying");
    Test *cleanMeUp = (*myTest);  // Refer to test obj
    deque<Test *>::iterator insertPos = mTests[tr.xLev][tr.yLev].erase(myTest);
    mTests[tr.xLev][tr.yLev].insert(insertPos, cleanMeUp->Clone());
    delete cleanMeUp;

    return result;
}


bool
Group::SkippingTest(TestRef &tr, vector<TestRef> &skipTest)
{
    for (size_t i = 0; i < skipTest.size(); i++) {
         if ((tr.group == skipTest[i].group) && (tr.xLev == skipTest[i].xLev) &&
             (tr.yLev == skipTest[i].yLev) && (tr.zLev == skipTest[i].zLev)) {

            LOG_NRM("Instructed to skip specific test: %ld:%ld.%ld.%ld",
                tr.group, tr.xLev, tr.yLev, tr.zLev);
            return true;

         } else if ((tr.group == skipTest[i].group) &&
             ((UINT_MAX == skipTest[i].xLev) ||
              (UINT_MAX == skipTest[i].yLev) ||
              (UINT_MAX == skipTest[i].zLev))) {

            LOG_NRM("Instructed to skip entire group: %ld", tr.group);
            return true;
         }
    }
    return false;
}


bool
Group::GetTestDependency(TestRef test, TestRef &cfgDepend,
    TestIteratorType &seqDepend)
{
    // Assume failure, ignore cfgDepend & seqDepend
    cfgDepend = test;
    seqDepend = -1;

    if (test.group == mGrpNum) {
        if ((test.yLev == 0) && (test.zLev == 0)) {
            LOG_NRM("Targeted test has zero dependencies");
            return true;
        } else if (test.zLev == 0) {
            cfgDepend.Init(test.group, test.xLev, 0, 0);
            if (TestExists(cfgDepend)) {
                LOG_NRM("Targeted test has a configuration dependency");
                return true;
            } else {
                LOG_ERR("Unable to locate configuration dependency");
                return false;
            }
        } else {
            TestRef seqTest(test.group, test.xLev, test.yLev, 0);
            if (TestRefToIterator(seqTest, seqDepend)) {
                LOG_NRM("Targeted test has a sequence dependency");
            } else {
                LOG_ERR("Unable to locate sequence test dependency");
                return false;
            }

            // There may or may not be a configuration dependency
            if (test.yLev != 0) {
                cfgDepend.Init(test.group, test.xLev, 0, 0);
                if (TestExists(cfgDepend)) {
                    LOG_NRM("Targeted test has a configuration dependency");
                    return true;
                } else {
                    LOG_ERR("Unable to locate configuration dependency");
                    return false;
                }
            }
            return true;
        }
    }
    LOG_ERR("Targeted test does not belong to this group: %ld", mGrpNum);
    return false;
}
