#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "dumpCtrlrAddrSpace_r10b.h"
#include "grpInformative.h"
#include "../globals.h"


DumpCtrlrAddrSpace_r10b::DumpCtrlrAddrSpace_r10b(int fd) : Test(fd, SPECREV_10b)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section n/a");
    mTestDesc.SetShort(     "Dump all controller address space registers");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Dumps the values of every controller address space register, offset "
        "from PCI BAR0/BAR1 address, to the file: " FILENAME_DUMP_CTRLR_REGS);
}


DumpCtrlrAddrSpace_r10b::~DumpCtrlrAddrSpace_r10b()
{
}


bool
DumpCtrlrAddrSpace_r10b::RunCoreTest()
{
    int fd;
    string work;
    uint64_t value = 0;
    const CtlSpcType *pciMetrics = gRegisters->GetCtlMetrics();


    // Dumping all register values to well known file
    if ((fd = open(FILENAME_DUMP_CTRLR_REGS, FILENAME_FLAGS,
        FILENAME_MODE)) == -1) {

        LOG_ERR("file=%s: %s", FILENAME_DUMP_CTRLR_REGS, strerror(errno));
        throw exception();
    }

    // Read all registers in ctrlr space
    for (int i = 0; i < CTLSPC_FENCE; i++) {
        if (pciMetrics[i].specRev != mSpecRev)
            continue;

        if (pciMetrics[i].size > MAX_SUPPORTED_REG_SIZE) {
            uint8_t *buffer;
            buffer = new uint8_t[pciMetrics[i].size];
            if (gRegisters->Read(NVMEIO_BAR01, pciMetrics[i].size,
                pciMetrics[i].offset, buffer) == false) {
                goto ERROR_OUT;
            } else {
                string work = "  ";
                work += gRegisters->FormatRegister(NVMEIO_BAR01,
                    pciMetrics[i].size, pciMetrics[i].offset, buffer);
                work += "\n";
                write(fd, work.c_str(), work.size());
            }
        } else if (pciMetrics[i].size > MAX_SUPPORTED_REG_SIZE) {
            continue;   // Don't care about really large areas, their reserved
        } else if (gRegisters->Read((CtlSpc)i, value) == false) {
            break;
        } else {
            work = "  ";    // indent reg values within each capability
            work += gRegisters->FormatRegister(pciMetrics[i].size,
                pciMetrics[i].desc, value);
            work += "\n";
            write(fd, work.c_str(), work.size());
        }
    }

    close(fd);
    return true;

ERROR_OUT:
    close(fd);
    throw exception();
}


