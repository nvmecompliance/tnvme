#ifndef _TESTCASE_r10b_H_
#define _TESTCASE_r10b_H_

#include "test.h"


/** \verbatim
 * -----------------------------------------------------------------------------
 * ----------------Mandatory rules for children to follow-----------------------
 * -----------------------------------------------------------------------------
 * 1) See notes in the header file of the Test base class
 * \endverbatim
 */
class TestCase_r10b : public Test
{
public:
    TestCase_r10b(int fd, string grpName, string testName);
    virtual ~TestCase_r10b();

    /**
     * IMPORTANT: Read Test::Clone() header comment.
     */
    virtual TestCase_r10b *Clone() const
        { return new TestCase_r10b(*this); }
    TestCase_r10b &operator=(const TestCase_r10b &other);
    TestCase_r10b(const TestCase_r10b &other);


protected:
    virtual bool RunCoreTest();


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator().
    ///////////////////////////////////////////////////////////////////////////
};


#endif
