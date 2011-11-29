#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "dumpPciAddrSpace_r10b.h"
#include "grpInformative.h"
#include "globals.h"
#include "../Utils/fileSystem.h"


DumpPciAddrSpace_r10b::DumpPciAddrSpace_r10b(int fd, string grpName,
    string testName) :
    Test(fd, grpName, testName, SPECREV_10b)
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section n/a");
    mTestDesc.SetShort(     "Dump all PCI address space registers");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Dumps the values of every PCI address space register to a file");
}


DumpPciAddrSpace_r10b::~DumpPciAddrSpace_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


DumpPciAddrSpace_r10b::
DumpPciAddrSpace_r10b(const DumpPciAddrSpace_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


DumpPciAddrSpace_r10b &
DumpPciAddrSpace_r10b::operator=(const DumpPciAddrSpace_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


bool
DumpPciAddrSpace_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) none
     *  \endverbatim
     */

    int fd;
    string work;
    uint64_t value;
    string outFile;
    const PciSpcType *pciMetrics = gRegisters->GetPciMetrics();
    const vector<PciCapabilities> *pciCap = gRegisters->GetPciCapabilities();


    // Dumping all register values to well known file
    outFile = FileSystem::PrepLogFile(mGrpName, mTestName, "ctrl", "registers");
    if ((fd = open(outFile.c_str(), FILENAME_FLAGS, FILENAME_MODE)) == -1) {
        LOG_ERR("file=%s: %s", outFile.c_str(), strerror(errno));
        throw exception();
    }

    // Traverse the PCI header registers
    work = "PCI header registers\n";
    write(fd, work.c_str(), work.size());
    for (int j = 0; j < PCISPC_FENCE; j++) {
        if (pciMetrics[j].specRev != mSpecRev)
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
            if (pciMetrics[j].specRev != mSpecRev)
                continue;

            if (pciCap->at(i) == pciMetrics[j].cap) {
                if (pciMetrics[j].size > MAX_SUPPORTED_REG_SIZE) {
                    bool err = false;
                    uint8_t *buffer;
                    buffer = new uint8_t[pciMetrics[j].size];

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
                    delete [] buffer;
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
    throw exception();
}


void
DumpPciAddrSpace_r10b::WriteToFile(int fd, const PciSpcType regMetrics,
    uint64_t value)
{
    string work = "  ";    // indent reg values within each capability
    work += gRegisters->FormatRegister(regMetrics.size,
        regMetrics.desc, value);
    work += "\n";
    write(fd, work.c_str(), work.size());
}


