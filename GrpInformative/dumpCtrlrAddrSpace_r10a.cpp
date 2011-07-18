#include "dumpCtrlrAddrSpace_r10a.h"
#include "grpInformative.h"


DumpCtrlrAddrSpace_r10a::DumpCtrlrAddrSpace_r10a(int fd) : Test(fd)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0a, section n/a");
    mTestDesc.SetShort(     "Dump all controller address space registers");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Dumps the values of every controller address space register, offset "
        "from PCI BAR0/BAR1 address, to the file: " FILENAME_DUMP_CTRLR_REGS);
}


DumpCtrlrAddrSpace_r10a::~DumpCtrlrAddrSpace_r10a()
{
}


bool
DumpCtrlrAddrSpace_r10a::RunCoreTest()
{
    return false;   // todo; add some reset logic when available
}


