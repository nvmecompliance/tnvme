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


Group::Group(size_t grpNum, string grpName, string desc)
{
    mGrpNum = grpNum;
    mGrpName = grpName;

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
Group::GetTestSet(TestRef &target, TestSetType &dependencies, int64_t &tstIdx)
{
    TestRef thisTest;

    dependencies.clear();
    tstIdx = -1;

    if (target.group != mGrpNum) {
        LOG_ERR("Targeted test does not belong to this group: %ld", mGrpNum);
        return false;
    } else if ((target.xLev != UINT_MAX) &&
               (target.yLev != UINT_MAX) &&
               (target.zLev != UINT_MAX)) {

        LOG_DBG("Requesting dependency test set for test %ld:%ld.%ld.%ld",
            target.group, target.xLev, target.yLev, target.zLev);
        if ((target.yLev == 0) && (target.zLev == 0)) {
            LOG_NRM("Targeted test has zero dependencies");

        } else if (target.zLev == 0) {
            // There is a configuration dependency for the targeted test
            thisTest.Init(target.group, target.xLev, 0, 0);
            if (TestExists(thisTest)) {
                LOG_NRM("Targeted test has a configuration dependency");
                LOG_DBG("Adding test: %s", thisTest.ToString().c_str());
                dependencies.push_back(thisTest);
            } else {
                LOG_ERR("Unable to locate configuration dependency");
                return false;
            }

        } else {
            // There is a sequence dependency for the targeted test
            if (target.yLev != 0) {
                // There is a config dependency in addition to a seq dependency
                thisTest.Init(target.group, target.xLev, 0, 0);
                if (TestExists(thisTest)) {
                    LOG_NRM("Targeted test has a configuration dependency");
                    LOG_DBG("Adding test: %s", thisTest.ToString().c_str());
                    dependencies.push_back(thisTest);
                } else {
                    LOG_ERR("Unable to locate configuration dependency");
                    return false;
                }
            }

            // Now add in all the sequence test dependencies; find the root
            // sequence test case first and traverse to the targeted test
            TestIteratorType seqIter;
            TestIteratorType targetIter;
            TestRef rootSeq(target.group, target.xLev, target.yLev, 0);
            if (TestRefToIterator(rootSeq, seqIter)) {
                LOG_NRM("Targeted test has a sequence dependency");
                if (TestRefToIterator(target, targetIter)) {
                    for (TestIteratorType i = seqIter; i < targetIter; i++) {
                        if (IteraterToTestRef(i, thisTest) == false) {
                            LOG_ERR("Unable to locate sequence dependency");
                            return false;
                        }
                        LOG_DBG("Adding test: %s", thisTest.ToString().c_str());
                        dependencies.push_back(thisTest);
                    }
                } else {
                    LOG_ERR("Unable to locate targeted test");
                    return false;
                }
            } else {
                LOG_ERR("Unable to locate sequence test dependency");
                return false;
            }
        }

        LOG_DBG("Adding test: %s", thisTest.ToString().c_str());
        dependencies.push_back(target);
    } else {
        LOG_DBG("Requesting dependency set for group %ld", mGrpNum);
        TestIteratorType tstIter = 0;
        while (IteraterToTestRef(tstIter++, thisTest)) {
            LOG_DBG("Adding test: %s", thisTest.ToString().c_str());
            dependencies.push_back(thisTest);
        }
    }

    tstIdx = 0;
    LOG_DBG("dependencies(size)=%ld, tstIdx=%ld", dependencies.size(), tstIdx);
    return true;
}


Group::TestResult
Group::RunTest(TestSetType &dependencies, int64_t &tstIdx,
    vector<TestRef> &skipTest, int64_t &numSkipped, bool preserve,
    vector<TestRef> &failedTests, vector<TestRef> &skippedTests)
{
    string work;
    numSkipped = 0;
    TestResult result = TR_FAIL;

    // Preliminary error checking
    if ((tstIdx >= (int64_t)dependencies.size()) || (tstIdx == -1)) {
        tstIdx = -1;
        return TR_NOTFOUND;
    }

    // Get a pointer to the test to execute
    TestRef tr = dependencies[tstIdx];
    if (TestExists(tr) == false) {
        tstIdx = -1;
        return TR_NOTFOUND;
    }
    deque<Test *>::iterator myTest = mTests[tr.xLev][tr.yLev].begin();
    advance(myTest, tr.zLev);

    // The first test within every group must be proceeded with a chance to
    // save the state of the DUT, if and only if the feature is enabled.
    if (gCmdLine.restore && (tstIdx == 0)) {
        LOG_NRM("Saving the state of the DUT");
        if (SaveState() == false) {
            LOG_ERR("Unable to save the state of the DUT");
            tstIdx = -1;
            return TR_FAIL;
        }
    }

    LOG_NRM("-----------------START TEST-----------------");
    FORMAT_GROUP_DESCRIPTION(work, this)
    LOG_NRM("%s", work.c_str());
    FORMAT_TEST_NUM(work, "", tr.xLev, tr.yLev, tr.zLev)
    work += (*myTest)->GetClassName();
    work += ": ";
    work += (*myTest)->GetShortDescription();
    LOG_NRM("%s", work.c_str());
    LOG_NRM("Compliance: %s", (*myTest)->GetComplianceDescription().c_str());
    LOG_NRM("%s", (*myTest)->GetLongDescription(false, 0).c_str());

    if (SkippingTest(tr, skipTest)) {
        result = TR_SKIPPING;
        skippedTests.push_back(tr);
        numSkipped = (AdvanceDependencies(dependencies, tstIdx, false,
            skippedTests) + 1);
    } else {
        switch ((*myTest)->Runnable(preserve)) {
        case Test::RUN_TRUE:
            result = (*myTest)->Run() ? TR_SUCCESS: TR_FAIL;
            if (result == TR_FAIL) {
                failedTests.push_back(tr);
                numSkipped = AdvanceDependencies(dependencies, tstIdx, true,
                    skippedTests);
            } else {
                if (++tstIdx >= (int64_t)dependencies.size())
                    tstIdx = -1;
                LOG_NRM("SUCCESSFUL test case run");
            }
            break;

        case Test::RUN_FALSE:
            result = TR_SKIPPING;
            skippedTests.push_back(tr);
            numSkipped = (AdvanceDependencies(dependencies, tstIdx, false,
                skippedTests) + 1);
            LOG_WARN("Reporting not runnable, skipping test: %ld:%ld.%ld.%ld",
                tr.group, tr.xLev, tr.yLev, tr.zLev);
            break;

        default:
        case Test::RUN_FAIL:
            failedTests.push_back(tr);
            numSkipped = AdvanceDependencies(dependencies, tstIdx, true,
                skippedTests);
            break;
        }
    }

    FORMAT_GROUP_DESCRIPTION(work, this)
    LOG_NRM("%s", work.c_str());
    FORMAT_TEST_NUM(work, "", tr.xLev, tr.yLev, tr.zLev)
    work += (*myTest)->GetClassName();
    work += ": ";
    work += (*myTest)->GetShortDescription();
    LOG_NRM("%s", work.c_str());
    LOG_NRM("------------------END TEST------------------");

    // The last test within every group or a test failure must be following
    // with a chance to restore the state of the DUT, if and only if the
    // feature is enabled.
    if (gCmdLine.restore && ((tstIdx == -1) || (result == TR_FAIL))) {
        LOG_NRM("Restoring the state of the DUT");
        if (RestoreState() == false) {
            LOG_ERR("Unable to restore the state of the DUT");
            tstIdx = -1;
            return TR_FAIL;
        }
    }

    // Guarantee nothing residing or unintended is left around. Enforce this
    // by destroying the existing test obj and replace it with a clone of
    // itself so looping tests over can still be supported.
    LOG_DBG("Enforcing test obj cleanup, cloning & destroying");
    Test *cleanMeUp = (*myTest);  // Refer to test obj
    deque<Test *>::iterator insertPos = mTests[tr.xLev][tr.yLev].erase(myTest);
    mTests[tr.xLev][tr.yLev].insert(insertPos, cleanMeUp->Clone());
    delete cleanMeUp;
    return result;
}


int64_t
Group::AdvanceDependencies(TestSetType &dependencies, int64_t &tstIdx,
    bool failed, vector<TestRef> &skippedTests)
{
    int64_t origTstIdx = tstIdx;
    int64_t work;

    // Preliminary error checking
    if ((tstIdx >= (int64_t)dependencies.size()) || (tstIdx == -1)) {
        tstIdx = -1;
        return 0;
    }
    TestRef dt = dependencies[tstIdx];      // DependentTest  (dt)

    if (++tstIdx >= (int64_t)dependencies.size()) {
        tstIdx = -1;
        return 0;
    }
    TestRef st = dependencies[tstIdx];      // SubsequentTest (st)

    if (failed) {
        // When a test fails, a FrmwkEx() is thrown and performs a
        // DISABLE_COMPLETELY. This is the most destructive action in the
        // framework, and thus everything subsequent of the xLev must be
        // considered dependents since no resource remains in the DUT
        while (dt.xLev == st.xLev) {
            skippedTests.push_back(st);
            if (++tstIdx >= (int64_t)dependencies.size()) {
                work = ((tstIdx - origTstIdx) - 1);
                tstIdx = -1;
                return work;
            }
            st = dependencies[tstIdx];
        }
    } else {
        // A failure didn't occur, something was skipped rather and thus
        // a complete destruction of resource did not occur. Considering
        // only those dependents which rely upon the now missing logic.
        if ((dt.yLev == 0) && (dt.zLev == 0)) {
            // All the following are dependents
            while (dt.xLev == st.xLev) {
                skippedTests.push_back(st);
                if (++tstIdx >= (int64_t)dependencies.size()) {
                    work = ((tstIdx - origTstIdx) - 1);
                    tstIdx = -1;
                    return work;
                }
                st = dependencies[tstIdx];
            }
        } else {
            // All the following are dependents
            while ((dt.xLev == st.xLev)  && (dt.yLev == st.yLev)){
                skippedTests.push_back(st);
                if (++tstIdx >= (int64_t)dependencies.size()) {
                    work = ((tstIdx - origTstIdx) - 1);
                    tstIdx = -1;
                    return work;
                }
                st = dependencies[tstIdx];
            }
        }
    }

    return (tstIdx - origTstIdx - 1);
}


bool
Group::SkippingTest(TestRef &tr, vector<TestRef> &skipTest)
{
    for (size_t i = 0; i < skipTest.size(); i++) {
         if ((tr.group == skipTest[i].group) && (tr.xLev == skipTest[i].xLev) &&
             (tr.yLev == skipTest[i].yLev) && (tr.zLev == skipTest[i].zLev)) {

            LOG_WARN("Instructed to skip specific test: %ld:%ld.%ld.%ld",
                tr.group, tr.xLev, tr.yLev, tr.zLev);
            return true;

         } else if ((tr.group == skipTest[i].group) &&
             ((UINT_MAX == skipTest[i].xLev) ||
              (UINT_MAX == skipTest[i].yLev) ||
              (UINT_MAX == skipTest[i].zLev))) {

            LOG_WARN("Instructed to skip entire group: %ld", tr.group);
            return true;
         }
    }
    return false;
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
    testIter = 0;

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


bool
SaveState()
{
    // For the majority of test groups this feature most likely won't be needed
    LOG_NRM("Saving state is intended to be over ridden in children");
    return true;
}


bool
RestoreState()
{
    // For the majority of test groups this feature most likely won't be needed
    LOG_NRM("Restoring state is intended to be over ridden in children");
    return true;
}
