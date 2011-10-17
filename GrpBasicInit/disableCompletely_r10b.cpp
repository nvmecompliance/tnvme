#include "disableCompletely_r10b.h"
#include "../globals.h"


DisableCompletely_r10b::DisableCompletely_r10b(int fd) : Test(fd, SPECREV_10b)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 7");
    mTestDesc.SetShort(     "Validate basic hardware initialization duties");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Setup for forthcoming tests. It will completely disabled the "
        "controller. All driver and NVME device memory is freed to the system, "
        "nothing will be operational, not even IRQ's");
}


DisableCompletely_r10b::~DisableCompletely_r10b()
{
}


bool
DisableCompletely_r10b::RunCoreTest()
{
    return true;
}
