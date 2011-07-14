#include "tnvme.h"
#include "group.h"

#define PAD_INDENT_LVL1         "    "
#define PAD_INDENT_LVL2         "      "
#define NEWLINE                 "\n"


Group::Group(size_t grpNum, SpecRevType specRev, string desc)
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

    for (size_t major = 0; major < mTests.size(); major++) {
        for (size_t minor = 0; minor < mTests[major].size(); minor++) {
            work += GetTestDescription(verbose, major, minor);
        }
    }

    return (work);
}


string
Group::GetTestDescription(bool verbose, size_t major, size_t minor)
{
    string work;

    FORMAT_TEST_NUM(work, PAD_INDENT_LVL1, major, minor)

    if (TestExists(major, minor) == false) {
        work += "unknown test";
        work += NEWLINE;

    } else {
        if (verbose) {
            work += mTests[major][minor]->GetShortDescription();
            work += NEWLINE;
            work += PAD_INDENT_LVL2;
            work += "Compliance: ";
            work += mTests[major][minor]->GetComplianceDescription();
            work += NEWLINE;
            work += mTests[major][minor]->
                GetLongDescription(true, sizeof(PAD_INDENT_LVL2) + 2);
        } else {
            work += mTests[major][minor]->GetShortDescription();
            work += NEWLINE;
        }
    }

    return (work);
}


bool
Group::TestExists(size_t major, size_t minor)
{
    if ((major >= mTests.size()) || (minor >= mTests[major].size())) {
        LOG_DBG("Test case %ld.%ld does not exist within group", major, minor);
        return false;
    }
    return true;
}


bool
Group::RunTest(TestIteratorType &testIter, bool &done)
{
    size_t count = 0;
    size_t major = 0;
    size_t minor = 0;
    done = false;

    while (count < testIter) {
        if (TestExists(major, minor+1)) {
            minor++;        // same group level, but the next test level count
            count++;
        } else if (TestExists(major+1, 0)) {
            major++;
            minor = 0;      // new group level, restart test level counting
            count++;
        } else {
            done = true;
            return true;
        }
    }

    if (count == testIter) {
        testIter++;     // next test to consider for execution in the future
        return RunTest(major, minor);
    }

    LOG_DBG("Programmatic failure; how can this occur?");
    return false;
}


bool
Group::RunTest(size_t major, size_t minor)
{
    string work;

    if (TestExists(major, minor) == false)
        return false;

    FORMAT_GROUP_DESCRIPTION(work, this)
    LOG_NORM("%s", work.c_str());

    FORMAT_TEST_NUM(work, "", major, minor)
    work += mTests[major][minor]->GetShortDescription();
    LOG_NORM("%s", work.c_str());
    LOG_NORM("Compliance: %s",
        mTests[major][minor]->GetComplianceDescription().c_str());
    LOG_NORM("%s",
        mTests[major][minor]->GetLongDescription(false, 0).c_str());

    return mTests[major][minor]->Run();
}
