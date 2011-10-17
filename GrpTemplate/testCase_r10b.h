#ifndef _TESTCASE_r10b_H_
#define _TESTCASE_r10b_H_

#include "test.h"


/**
 * The purpose of this class resides in the constructor
 */
class TestCase_r10b : public Test
{
public:
    TestCase_r10b(int fd);
    virtual ~TestCase_r10b();


protected:
    virtual bool RunCoreTest();


private:
};


#endif
