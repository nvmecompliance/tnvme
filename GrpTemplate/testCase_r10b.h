#ifndef _TESTCASE_r10b_H_
#define _TESTCASE_r10b_H_

#include "test.h"


/**
 * -----------------------------------------------------------------------------
 * ----------------Mandatory rules for children to follow-----------------------
 * -----------------------------------------------------------------------------
 * 1) See notes in the header file of the Test base class
 */
class TestCase_r10b : public Test
{
public:
    TestCase_r10b(int fd);
    virtual ~TestCase_r10b();

    virtual TestCase_r10b *Clone() const
        { return new TestCase_r10b(*this); }


protected:
    virtual bool RunCoreTest();


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Think carefully, see Test::Clone() hdr comment
    // Adding a member functions is fine.
    ///////////////////////////////////////////////////////////////////////////
};


#endif
