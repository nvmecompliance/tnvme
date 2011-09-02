#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "dumpPciAddrSpace_r10b.h"
#include "grpInformative.h"
#include "../globals.h"


DumpPciAddrSpace_r10b::DumpPciAddrSpace_r10b(int fd) : Test(fd)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section n/a");
    mTestDesc.SetShort(     "Dump all PCI address space registers");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Dumps the values of every PCI address space register to the file: "
        FILENAME_DUMP_PCI_REGS);
}


DumpPciAddrSpace_r10b::~DumpPciAddrSpace_r10b()
{
}


bool
DumpPciAddrSpace_r10b::RunCoreTest()
{
    int fd;
    string work;
    unsigned long long value;
    const PciSpcType *pciMetrics = gRegisters->GetPciMetrics();
    const vector<PciCapabilities> *pciCap = gRegisters->GetPciCapabilities();


    // Dumping all register values to well known file
    if ((fd = open(FILENAME_DUMP_PCI_REGS, FILENAME_FLAGS,
        FILENAME_MODE)) == -1) {

        LOG_ERR("file=%s: %s", FILENAME_DUMP_PCI_REGS, strerror(errno));
        return false;
    }

    // Traverse the PCI header registers
    work = "PCI header registers\n";
    write(fd, work.c_str(), work.size());
    for (int j = 0; j < PCISPC_FENCE; j++) {
        if (pciMetrics[j].specRev != SPECREV_10b)
            continue;

        // All PCI hdr regs don't have an associated capability
        if (pciMetrics[j].cap == PCICAP_FENCE) {
            if (gRegisters->Read((PciSpc)j, value) == false)
                goto ERROR_OUT;
            WriteToFile(fd, pciMetrics[j], value);
        }
    }

    // Traverse all discovered capabilities
    for (size_t i = 0; i < pciCap->size(); i++) {
        switch (pciCap->at(i)) {

        case PCICAP_PMCAP:
            work = "Capabilities: PMCAP: PCI power management\n";
            break;
        case PCICAP_MSICAP:
            work = "Capabilities: MSICAP: Message signaled interrupt\n";
            break;
        case PCICAP_MSIXCAP:
            work = "Capabilities: MSIXCAP: Message signaled interrupt ext'd\n";
            break;
        case PCICAP_PXCAP:
            work = "Capabilities: PXCAP: Message signaled interrupt\n";
            break;
        case PCICAP_AERCAP:
            work = "Capabilities: AERCAP: Advanced Error Reporting\n";
            break;
        default:
            LOG_ERR("PCI space reporting an unknown capability: %d\n",
                pciCap->at(i));
            goto ERROR_OUT;
        }
        write(fd, work.c_str(), work.size());

        // Read all registers assoc with the discovered capability
        for (int j = 0; j < PCISPC_FENCE; j++) {
            if (pciMetrics[j].specRev != SPECREV_10b)
                continue;

            if (pciCap->at(i) == pciMetrics[j].cap) {
                if (pciMetrics[j].size > MAX_SUPPORTED_REG_SIZE) {
                    bool err = false;
                    unsigned char *buffer;
                    buffer = new unsigned char[pciMetrics[j].size];

                    if (gRegisters->Read(NVMEIO_PCI_HDR, pciMetrics[j].size,
                        pciMetrics[j].offset, buffer) == false) {
                        err = true;
                    } else {
                        string work = "  ";
                        work += gRegisters->FormatRegister(NVMEIO_PCI_HDR,
                            pciMetrics[j].size, pciMetrics[j].offset, buffer);
                        work += "\n";
                        write(fd, work.c_str(), work.size());
                    }
                    delete buffer;
                    if (err)
                        goto ERROR_OUT;
                } else if (gRegisters->Read((PciSpc)j, value) == false) {
                    goto ERROR_OUT;
                } else {
                    WriteToFile(fd, pciMetrics[j], value);
                }
            }
        }
    }

    close(fd);
    return true;

ERROR_OUT:
    close(fd);
    return false;
}


void
DumpPciAddrSpace_r10b::WriteToFile(int fd, const PciSpcType regMetrics,
    unsigned long long value)
{
    string work = "  ";    // indent reg values within each capability
    work += gRegisters->FormatRegister(regMetrics.size,
        regMetrics.desc, value);
    work += "\n";
    write(fd, work.c_str(), work.size());
}


