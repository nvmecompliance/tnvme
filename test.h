#ifndef _TEST_H_
#define _TEST_H_

#include <string>
#include "testDescribe.h"

#define TD_MAX_CHAR_PER_LINE    72  // Allows for external formatting

using namespace std;


/**
* This class is the base/interface class for all individual test cases.
* This class purposely enforces children to conform and inherit certain
* functionality to handle mundane tasks.
*/
class Test
{
public:
    Test() {}
    virtual ~Test() {}

    /**
     * Get test information
     * @return Test description, short form.
     */
    string GetShortDescription() { return mTestDesc.GetShort(); }

    /**
     * Get test information
     * @param limit80Chars Pass whether to format the long paragraph to 80
     *          character column displays.
     * @param indent Pass num of spaces to indent the long paragraph
     * @return Test description, long form.
     */
    string GetLongDescription(bool limit80Chars, int indent)
        { return mTestDesc.GetLong(limit80Chars, indent); }

    /**
     * Get test information
     * @return Test compliance statement.
     */
    string GetComplianceDescription() { return mTestDesc.GetCompliance(); }

    /**
     * Run the test case.
     * @return true upon success, otherwise false.
     */
    bool Run();


protected:
    /// Children must populate this member, probably during construction
    TestDescribe mTestDesc;

    /**
     * Forcing children to implement the core logic of each test case.
     * @return true upon success, otherwise false.
     */
    virtual bool RunCoreTest() = 0;
};


#endif
