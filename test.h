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

#ifndef _TEST_H_
#define _TEST_H_

#include <string>
#include <exception>
#include "testDescribe.h"
#include "Singletons/registers.h"
#include "tnvme.h"


/**
* This class is the base/interface class for all individual test cases.
* This class purposely enforces children to conform and inherit certain
* functionality to handle mundane tasks.
*
* -----------------------------------------------------------------------------
* ----------------Mandatory rules for children to follow-----------------------
* -----------------------------------------------------------------------------
* 1) During construction populate member Test::mTestDesc
* 2) Utilize class RsrcMngr to create objects to pass to subsequent tests which
*    are all part of the same group.
* 3) Heap allocations taken during test execution, and not under the control
*    of the RsrcMngr, must be deleted in destructor.
* 4) The creation of member variables is allowed but special attention to
*    copy construction and operator=() must be implemented in all children.
*    See the comment in Test::Clone() for complete details.
* 5) Execute dbgMemLeak.sh to find any memory leaks as a result of adding and
*    running any new test cases.
* -----------------------------------------------------------------------------
*
* @note This class will not throw exceptions.
*/
class Test
{
public:
    /**
     * @param grpName Pass the name of the group to which this test belongs
     * @param testName Pass the name of the child class
     * @param specRev Provide the nvme spec rev. which is being targeted
     * @param errRegs Pass the reg bitmask to indicate which errors need to be
     *          flagged after each test completes execution.
     */
    Test(string grpName, string testName, SpecRev specRev);

    /**
     * Child derived Test:: objects are responsible for freeing all heap
     * allocations back to the system in the destructor which are not under
     * the control of the RsrcMngr.
     */
    virtual ~Test();

    /**
     * Get the C++ object name assigned to this object. This can be used to
     * locate the source code of a failed test case.
     * @return The C++ assigned name of this test object
     */
    string GetClassName() { return mTestName; }

    /**
     * Get test information
     * @return Test description, short form.
     */
    string GetShortDescription() { return mTestDesc.GetShort(); }

    /**
     * Get test information
     * @param limit80Chars Pass whether to format the long paragraph to 80
     *        character column displays.
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
     * Run the test case. This method will catch all exceptions and convert
     * them into a return value, thus children implementing RunCoreTest() are
     * allowed to throw exceptions and they are treated as errors.
     * @return true upon success, otherwise false.
     */
    bool Run();

    typedef enum {
        RUN_TRUE,       // Test is runnable and should be run
        RUN_FALSE,      // Test is not runnable, this is not an error
        RUN_FAIL        // Test is not runnable, this is an error, stop testing
    } RunType;

    /**
     * Is this test runnable? This method will catch all exceptions and convert
     * them into a return value, thus children implementing RunnableCoreTest()
     * are allowed to throw exceptions and are treated as returning false.
     * @note A test may not be runnable if the test is inherently destructive
     * and the preserve param forces non-destructive testing. A test may not be
     * runnable if say there are not enough resources on the device to perform
     * the tests and to make more resources that will require to be destructive
     * to the device and the preserve param forces non-destructive testing. y
     * @param preserve Pass true if the DUT's state must be preserved, false
     *        indicates the test is allowed to be destructive and change the
     *        permanent state/configuration of the DUT.
     * @return The appropriate value as determined dynamically during
     *         runtime evaluation.
     */
    virtual RunType Runnable(bool preserve);

    /**
     * Cloning objects are necessary to support automated resource cleanup
     * between subsequent test case runs. This is the cleanup action of test
     * lifetimes. Objects are cloned, then freed after each test completes to
     * force resource cleanup. This cloning uses special copy construction and
     * operator=() to achieve proper resource cleanup. The end result of a
     * clone action should be to re-create a new object w/o doing any copying of
     * pointers. Do not do shallow or deep copies of any pointer. Any pointer
     * is one of the following: share_ptr, weak_ptr, or C++ pointers using the
     * new operator or malloc(). All cloned objects must achieve NULL pointers
     * or resource cleanup will not be possible. Thus default copy constructors
     * and operator=() cannot be used because they do bitwise copying. Thus all
     * children must implement copy constructors and operator=() to achieve this
     * requirement to allow proper resource cleanup.
     */
    virtual Test *Clone() const { return new Test(*this); }
    Test &operator=(const Test &other);
    Test(const Test &other);


protected:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator().
    ///////////////////////////////////////////////////////////////////////////
    /// NVME spec rev being targeted
    SpecRev mSpecRev;
    /// The name of the parent container object, i.e. the group name
    string mGrpName;
    /// The name of the child
    string mTestName;
    /// Children must populate this during construction
    TestDescribe mTestDesc;

    /**
     * Forcing children to implement the core logic of each test case.
     * @return Nothing; children are forced to throw exceptions derived from
     *         class FrmwkEx() when a test failure occurs.
     */
    virtual void RunCoreTest();

    /**
     * Forcing children to implement the core logic of whether or not a test
     * is runnable.
     * @param preserve Pass true if the DUT's state must be preserved, false
     *        indicates the test is allowed to be destructive and change the
     *        state/config of the DUT.
     * @return The appropriate value as determined dynamically during
     *         runtime evaluation.
     */
    virtual RunType RunnableCoreTest(bool preserve);

    /**
     * Resets the sticky error bits of the PCI address space. Prior errors
     * should not cause errors in subsequent tests, resetting to avoid
     * incorrectly detected errors.
     */
    void ResetStatusRegErrors();

    /**
     * Check PCI and ctrl'r registers status registers for errors which may
     * be present.
     */
    bool GetStatusRegErrors();

    /**
     * Report bit position of val which is not like expectedVal
     * @param val Pass value to search against for inequality
     * @param expectedVal Pass the value to compare against for correctness
     * @return INT_MAX if they are equal, otherwise the bit position that isn't
     */
    int ReportOffendingBitPos(uint64_t val, uint64_t expectedVal);


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator().
    ///////////////////////////////////////////////////////////////////////////

    Test();
};


#endif
