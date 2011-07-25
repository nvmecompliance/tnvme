#include "tnvme.h"
#include "group.h"

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

        LOG_DBG("Test case %ld:%ld.%ld does not exist within group %ld",
            tr.group, tr.major, tr.minor, mGrpNum);
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


bool
Group::RunTest(TestIteratorType &testIter, vector<TestRef> &skipTest,
    bool &done)
{
    TestRef tr;
    done = false;

    if (IteraterToTestRef(testIter, tr) == false) {
        done = true;
        return true;
    }

    testIter++;     // next test to consider for execution in the future
    return RunTest(tr, skipTest);
}


bool
Group::RunTest(TestRef &tr, vector<TestRef> &skipTest)
{
    string work;

    if (TestExists(tr) == false)
        return false;

    FORMAT_GROUP_DESCRIPTION(work, this)
    LOG_NRM("%s", work.c_str());

    FORMAT_TEST_NUM(work, "", tr.major, tr.minor)
    work += mTests[tr.major][tr.minor]->GetShortDescription();
    LOG_NRM("%s", work.c_str());
    LOG_NRM("Compliance: %s",
        mTests[tr.major][tr.minor]->GetComplianceDescription().c_str());
    LOG_NRM("%s",
        mTests[tr.major][tr.minor]->GetLongDescription(false, 0).c_str());

    if (SkippingTest(tr, skipTest))
        return true;
    else
        return mTests[tr.major][tr.minor]->Run();
}


bool
Group::SkippingTest(TestRef &tr, vector<TestRef> &skipTest)
{
    for (size_t i = 0; i < skipTest.size(); i++) {
         if ((tr.group == skipTest[i].group) &&
             (tr.major == skipTest[i].major) &&
             (tr.minor == skipTest[i].minor)) {

            LOG_NRM("Instructed to skip test: %ld:%ld.%ld", tr.group, tr.major,
                tr.minor);
            return true;
         }
    }
    return false;
}
