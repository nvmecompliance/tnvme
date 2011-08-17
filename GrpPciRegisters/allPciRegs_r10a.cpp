#include "allPciRegs_r10a.h"


AllPciRegs_r10a::AllPciRegs_r10a(int fd) : Test(fd)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0a, section 3");
    mTestDesc.SetShort(     "Validate all PCI registers syntactically");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Validates the following; the RO fields cannot be written; all ASCII "
        "fields only contain chars 0x20 to 0x7e.");
}


AllPciRegs_r10a::~AllPciRegs_r10a()
{
}


bool
AllPciRegs_r10a::RunCoreTest()
{
    LOG_ERR("Test not implemented");
    return false;   // todo; add some reset logic when available
}


