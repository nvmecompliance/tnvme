#ifndef _GROUP_H_
#define _GROUP_H_

#include <string>
#include <vector>
#include "tnvme.h"
#include "test.h"

using namespace std;

/// Use to append a new x.0 test number at the group level
#define APPEND_TEST_AT_GROUP_LEVEL(test, fd)        \
    {                                               \
        vector<Test *> tmp;                         \
        tmp.push_back(new test(fd));                \
        mTests.push_back(tmp);                      \
    }

/// Use to append a new x.y test number at the last appended group level
#define APPEND_TEST_AT_LAST_LEVEL(test)     mTests.back().push_back(new test());

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
*/
class Group
{
public:
    /**
     * @param grpNum Pass the assigned group number, globally unique ID
     * @param specRev Pass which compliance is needed to target
     * @param desc Pass a 1-line comment describing group purpose
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

    /**
     * Run a spec'd test case and report back.
     * @param testIter Pass the test case iterator
     * @param skipTest Pass the complete list of test which should be skipped
     * @param done Returns true when testIter is pointing past the last possible
     *          test case, i.e. all tests have run, otherwise false indicates
     *          there could be more tests to execute within this group
     * @return true upon success, otherwise false
     */
    bool RunTest(TestIteratorType &testIter, vector<TestRef> &skipTest,
        bool &done);

    /**
     * Run a spec'd test case and report back.
     * @param tr Pass the test case number to execute
     * @param skipTest Pass the complete list of test which should be skipped
     * @return true upon success, otherwise false
     */
    bool RunTest(TestRef &tr, vector<TestRef> &skipTest);


protected:
    size_t      mGrpNum;
    string      mGrpDesc;
    SpecRev     mSpecRev;

    /// vector[major][minor];
    /// major test number: are related at the group level; 1.0, 2.0, 3.0
    /// minor test number: are related at the test level; x.1, x.2, x.3
    vector<vector<Test *> > mTests;

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
