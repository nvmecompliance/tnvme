/*
 * Copyright (c) 2011, Intel Corporation.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "kernelAPI.h"
#include "globals.h"

#define FILENAME_FLAGS         (O_RDWR | O_TRUNC | O_CREAT)
#define FILENAME_MODE          (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)


KernelAPI::KernelAPI()
{
}


KernelAPI::~KernelAPI()
{
}


bool
KernelAPI::SoftReset()
{
    bool retVal;

    // In user space, in kernel space and in hardware, nothing remains.
    LOG_NRM("Performing soft reset");
    gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY);
    if ((retVal = gCtrlrConfig->SetIrqScheme(INT_NONE, 0)) == false)
        LOG_ERR("Setting IRQ scheme failed");
    return retVal;
}


uint8_t *
KernelAPI::mmap(int fd, size_t bufLength, uint16_t bufID, MmapRegion region)
{
    int prot = PROT_READ;

    if (region >= MMAPREGION_FENCE) {
        LOG_DBG("Detected illegal region = %d", region);
        throw exception();
    }

    if (region == MMR_META)
        prot |= PROT_WRITE;

    off_t encodeOffset = bufID;
    encodeOffset |= ((off_t)region << METADATA_UNIQUE_ID_BITS);
    encodeOffset *= sysconf(_SC_PAGESIZE);
    return (uint8_t *)::mmap(0, bufLength, prot, MAP_SHARED, fd, encodeOffset);
}


void
KernelAPI::munmap(uint8_t *memPtr, size_t bufLength)
{
    ::munmap(memPtr, bufLength);
}


void
KernelAPI::DumpKernelMetrics(int fd, LogFilename filename)
{
    int rc;

    struct nvme_file dumpMe = { filename.length(), filename.c_str() };

    LOG_NRM("Dump dnvme metrics to filename: %s", filename.c_str());
    if ((rc = ioctl(fd, NVME_IOCTL_DUMP_METRICS, &dumpMe)) < 0) {
        LOG_DBG("Unable to dump dnvme metrics, error code = %d", rc);
        throw exception();
    }
}


void
KernelAPI::DumpCtrlrSpaceRegs(SpecRev specRev, LogFilename filename)
{
    int fd;
    string work;
    uint64_t value = 0;
    const CtlSpcType *pciMetrics = gRegisters->GetCtlMetrics();


    // Dumping all register values to well known file
    if ((fd = open(filename.c_str(), FILENAME_FLAGS, FILENAME_MODE)) == -1) {
        LOG_ERR("file=%s: %s", filename.c_str(), strerror(errno));
        throw exception();
    }

    // Read all registers in ctrlr space
    for (int i = 0; i < CTLSPC_FENCE; i++) {
        if (pciMetrics[i].specRev != specRev)
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
            delete [] buffer;
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
    return;

ERROR_OUT:
    close(fd);
    throw exception();
}


void
KernelAPI::DumpPciSpaceRegs(SpecRev specRev, LogFilename filename)
{
    int fd;
    string work;
    uint64_t value;
    const PciSpcType *pciMetrics = gRegisters->GetPciMetrics();
    const vector<PciCapabilities> *pciCap = gRegisters->GetPciCapabilities();


    // Dumping all register values to well known file
    if ((fd = open(filename.c_str(), FILENAME_FLAGS, FILENAME_MODE)) == -1) {
        LOG_ERR("file=%s: %s", filename.c_str(), strerror(errno));
        throw exception();
    }

    // Traverse the PCI header registers
    work = "PCI header registers\n";
    write(fd, work.c_str(), work.size());
    for (int j = 0; j < PCISPC_FENCE; j++) {
        if (pciMetrics[j].specRev != specRev)
            continue;

        // All PCI hdr regs don't have an associated capability
        if (pciMetrics[j].cap == PCICAP_FENCE) {
            if (gRegisters->Read((PciSpc)j, value) == false)
                goto ERROR_OUT;
            RegToFile(fd, pciMetrics[j], value);
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
            if (pciMetrics[j].specRev != specRev)
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
                    RegToFile(fd, pciMetrics[j], value);
                }
            }
        }
    }

    close(fd);
    return;

ERROR_OUT:
    close(fd);
    throw exception();
}


void
KernelAPI::RegToFile(int fd, const PciSpcType regMetrics, uint64_t value)
{
    string work = "  ";    // indent reg values within each capability
    work += gRegisters->FormatRegister(regMetrics.size,
        regMetrics.desc, value);
    work += "\n";
    write(fd, work.c_str(), work.size());
}


void
KernelAPI::LogCQMetrics(struct nvme_gen_cq &cqMetrics)
{
    LOG_NRM("CQMetrics.q_id           = 0x%04X", cqMetrics.q_id);
    LOG_NRM("CQMetrics.tail_ptr       = 0x%04X", cqMetrics.tail_ptr);
    LOG_NRM("CQMetrics.head_ptr       = 0x%04X", cqMetrics.head_ptr);
    LOG_NRM("CQMetrics.elements       = 0x%04X", cqMetrics.elements);
    LOG_NRM("CQMetrics.irq_enabled    = 0x%02X", cqMetrics.irq_enabled);
    LOG_NRM("CQMetrics.irq_no         = 0x%04X", cqMetrics.irq_no);
    LOG_NRM("CQMetrics.int_vec        = 0x%04X", cqMetrics.int_vec);
    LOG_NRM("CQMetrics.pbit_new_entry = 0x%02X", cqMetrics.pbit_new_entry);
}


void
KernelAPI::LogSQMetrics(struct nvme_gen_sq &sqMetrics)
{
    LOG_NRM("CQMetrics.sq_id          = 0x%04X", sqMetrics.sq_id);
    LOG_NRM("CQMetrics.cq_id          = 0x%04X", sqMetrics.cq_id);
    LOG_NRM("CQMetrics.tail_ptr       = 0x%04X", sqMetrics.tail_ptr);
    LOG_NRM("CQMetrics.tail_ptr_virt  = 0x%04X", sqMetrics.tail_ptr_virt);
    LOG_NRM("CQMetrics.head_ptr       = 0x%04X", sqMetrics.head_ptr);
    LOG_NRM("CQMetrics.elements       = 0x%04X", sqMetrics.elements);
}

