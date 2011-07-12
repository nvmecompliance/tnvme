#include "tnvme.h"
#include "test.h"


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

