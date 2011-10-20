#include "testCase_r10b.h"
#include "../globals.h"


TestCase_r10b::TestCase_r10b(int fd) : Test(fd, SPECREV_10b)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section ?");
    mTestDesc.SetShort(     "This is a template test");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "This test is a template for future use. This string description has "
        "no length restrictions. See header file for class TestDescribe for "
        "further details.c");
}


TestCase_r10b::~TestCase_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


bool
TestCase_r10b::RunCoreTest()
{
    return true;
}
