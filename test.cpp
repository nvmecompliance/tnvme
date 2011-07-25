#include "tnvme.h"
#include "test.h"


Test::Test(int fd)
{
    mFd = fd;
    if (mFd < 0)
        LOG_DBG("Object created with a bad FD=%d", fd);
}


Test::~Test()
{
}


bool
Test::Run()
{
    if (RunCoreTest()) {
        LOG_NRM("SUCCESSFUL test case run");
        return true;
    } else {
        LOG_NRM("FAILED test case run");
        return false;
    }
}

