#include "allCtrlRegs_r10b.h"


AllCtrlRegs_r10b::AllCtrlRegs_r10b(int fd) : Test(fd)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0a, section 3");
    mTestDesc.SetShort(     "Validate all controller registers syntactically");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Validates the following; the RO fields which are not implementation "
        "specific contain default values; The RO fields cannot be written; All "
        "ASCII fields only contain chars 0x20 to 0x7e.");
}


AllCtrlRegs_r10b::~AllCtrlRegs_r10b()
{
}


bool
AllCtrlRegs_r10b::RunCoreTest()
{
    LOG_ERR("Test not implemented");
    return false;   // todo; add some reset logic when available
}


