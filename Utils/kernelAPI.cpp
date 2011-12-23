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

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "kernelAPI.h"
#include "globals.h"


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

