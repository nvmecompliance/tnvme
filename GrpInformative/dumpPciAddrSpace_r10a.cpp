#include "dumpPciAddrSpace_r10a.h"
#include "grpInformative.h"


DumpPciAddrSpace_r10a::DumpPciAddrSpace_r10a(int fd) : Test(fd)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0a, section n/a");
    mTestDesc.SetShort(     "Dump all PCI address space registers");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Dumps the values of every PCI address space register to the file: "
        FILENAME_DUMP_PCI_REGS);
}


DumpPciAddrSpace_r10a::~DumpPciAddrSpace_r10a()
{
}


bool
DumpPciAddrSpace_r10a::RunCoreTest()
{
    return false;   // todo; add some reset logic when available
}


