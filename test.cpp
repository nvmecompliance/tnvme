#include "tnvme.h"
#include "test.h"


Test::Test(int fd)
{
    mFd = fd;
}


Test::~Test()
{
}


bool
Test::Run()
{
    if (RunCoreTest()) {
        LOG_NORM("SUCCESS");
        return (true);
    } else {
        LOG_NORM("FAILED");
        return (false);
    }
}

