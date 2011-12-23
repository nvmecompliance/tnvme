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

#include "sq.h"
#include "globals.h"
#include "../Utils/kernelAPI.h"


SQ::SQ() :
    Queue(0, Trackable::OBJTYPE_FENCE)
{
    // This constructor will throw
}


SQ::SQ(int fd, Trackable::ObjType objBeingCreated) :
    Queue(fd, objBeingCreated)
{
    mCqId = 0;
}


SQ::~SQ()
{
    try {
        // Cleanup duties for this Q's buffer
        if (GetIsContig()) {
            // Contiguous memory is alloc'd and owned by the kernel
            KernelAPI::munmap(mContigBuf, GetQSize());
        }
    } catch (...) {
        ;   // Destructors should never throw. If the object is deleted B4
            // it is Init'd() properly, it could throw, so catch and ignore
    }
}


void
SQ::Init(uint16_t qId, uint16_t entrySize, uint16_t numEntries,
    uint16_t cqId)
{
    Queue::Init(qId, entrySize, numEntries);
    mCqId = cqId;


    LOG_NRM("Allocating contiguous SQ memory in dnvme");
    if (numEntries < 2)
        LOG_NRM("WARNING: Number elements breaches spec requirement");

    if (GetIsAdmin()) {
        if (gCtrlrConfig->IsStateEnabled()) {
            // At best this will cause tnvme to seg fault or a kernel crash
            // The NVME spec states unpredictable outcomes will occur.
            LOG_DBG("Creating an ASQ while ctrlr is enabled is a shall not");
            throw exception();
        }

        // We are creating a contiguous ASQ. ASQ's have a constant well known
        // element size and no setup is required for this type of Q.
        int ret;
        struct nvme_create_admn_q q;
        q.elements = GetNumEntries();
        q.type = ADMIN_SQ;

        LOG_NRM("Init contig ASQ: (id, cqid, entrySize, numEntries) = "
            "(%d, %d, %d, %d)", GetQId(), GetCqId(), GetEntrySize(),
            GetNumEntries());

        if ((ret = ioctl(mFd, NVME_IOCTL_CREATE_ADMN_Q, &q)) < 0) {
            LOG_DBG("Q Creation failed by dnvme with error: 0x%02X", ret);
            throw exception();
        }
    } else {
        // We are creating a contiguous IOSQ.
        struct nvme_prep_sq q;
        q.sq_id = GetQId();
        q.cq_id = GetCqId();
        q.elements = GetNumEntries();
        q.contig = true;
        CreateIOSQ(q);
    }

    // Contiguous Q's are created in dnvme and must be mapped back to user space
    mContigBuf = KernelAPI::mmap(mFd, GetQSize(), GetQId(), KernelAPI::MMR_SQ);
    if (mContigBuf == NULL) {
        LOG_DBG("Unable to mmap contig memory to user space");
        throw exception();
    }
}


void
SQ::Init(uint16_t qId, uint16_t entrySize, uint16_t numEntries,
    const SharedMemBufferPtr memBuffer, uint16_t cqId)
{
    Queue::Init(qId, entrySize, numEntries);
    mCqId = cqId;


    LOG_NRM("Allocating discontiguous SQ memory in tnvme");
    if (numEntries < 2)
        LOG_NRM("WARNING: Number elements breaches spec requirement");

    if (memBuffer == MemBuffer::NullMemBufferPtr) {
        LOG_DBG("Passing an uninitialized SharedMemBufferPtr");
        throw exception();
    } else if (GetIsAdmin()) {
        // There are no appropriate methods for an NVME device to report ASC/ACQ
        // creation errors, thus since ASC/ASQ may only be contiguous then don't
        // allow these problems to be injected, at best they will only succeed
        // to seg fault the app or crash the kernel.
        LOG_DBG("Illegal memory alignment will corrupt");
        throw exception();
    } else  if (memBuffer->GetBufSize() < GetQSize()) {
        LOG_DBG("Q buffer memory ambiguous to passed size params");
        LOG_DBG("Mem buffer size = %d, Q size = %d", memBuffer->GetBufSize(),
            GetQSize());
        throw exception();
    } else if (memBuffer->GetAlignment() != sysconf(_SC_PAGESIZE)) {
        // Nonconformance to page alignment will seg fault the app or crash
        // the kernel. This state is not testable since no errors can be
        // reported by hdw, thus disallow this attempt.
        LOG_DBG("Q content memory shall be page aligned");
        throw exception();
    }

    // Zero out the content memory so the P-bit correlates to a newly alloc'd Q.
    // Also assuming life time ownership of this object if it wasn't created
    // by the RsrcMngr.
    mDiscontigBuf = memBuffer;
    mDiscontigBuf->Zero();

    // We are creating a discontiguous IOSQ
    struct nvme_prep_sq q;
    q.sq_id = GetQId();
    q.cq_id = GetCqId();
    q.elements = GetNumEntries();
    q.contig = false;
    CreateIOSQ(q);
}


void
SQ::CreateIOSQ(struct nvme_prep_sq &q)
{
    int ret;

    LOG_NRM("Init %s SQ: (id, cqid, entrySize, numEntries) = (%d, %d, %d, %d)",
        q.contig ? "contig" : "discontig", GetQId(), GetCqId(), GetEntrySize(),
        GetNumEntries());

    if ((ret = ioctl(mFd, NVME_IOCTL_PREPARE_SQ_CREATION, &q)) < 0) {
        LOG_DBG("Q Creation failed by dnvme with error: 0x%02X", ret);
        throw exception();
    }
}


struct nvme_gen_sq
SQ::GetQMetrics()
{
    int ret;
    struct nvme_gen_sq qMetrics;
    struct nvme_get_q_metrics getQMetrics;

    getQMetrics.q_id = GetQId();
    getQMetrics.type = METRICS_SQ;
    getQMetrics.nBytes = sizeof(qMetrics);
    getQMetrics.buffer = (uint8_t *)&qMetrics;

    if ((ret = ioctl(mFd, NVME_IOCTL_GET_Q_METRICS, &getQMetrics)) < 0) {
        LOG_DBG("Get Q metrics failed by dnvme with error: 0x%02X", ret);
        throw exception();
    }
    return qMetrics;
}


union SE
SQ::PeekSE(uint16_t indexPtr)
{
    union SE *dataPtr;

    if (GetIsContig())
        dataPtr = (union SE *)mContigBuf;
    else
        dataPtr = (union SE *)mDiscontigBuf->GetBuffer();

    for (int i = 0; i < GetNumEntries(); i++, dataPtr++) {
        if (i == indexPtr)
            return *dataPtr;
    }

    LOG_DBG("Unable to locate index within Q");
    throw exception();
}


void
SQ::LogSE(uint16_t indexPtr)
{
    union SE se = PeekSE(indexPtr);
    LOG_NRM("Logging Submission Element (SE)...");
    LOG_NRM("SQ %d, SE %d, DWORD0:  0x%08X", GetQId(), indexPtr, se.d.dw0);
    LOG_NRM("SQ %d, SE %d, DWORD1:  0x%08X", GetQId(), indexPtr, se.d.dw1);
    LOG_NRM("SQ %d, SE %d, DWORD2:  0x%08X", GetQId(), indexPtr, se.d.dw2);
    LOG_NRM("SQ %d, SE %d, DWORD3:  0x%08X", GetQId(), indexPtr, se.d.dw3);
    LOG_NRM("SQ %d, SE %d, DWORD4:  0x%08X", GetQId(), indexPtr, se.d.dw4);
    LOG_NRM("SQ %d, SE %d, DWORD5:  0x%08X", GetQId(), indexPtr, se.d.dw5);
    LOG_NRM("SQ %d, SE %d, DWORD6:  0x%08X", GetQId(), indexPtr, se.d.dw6);
    LOG_NRM("SQ %d, SE %d, DWORD7:  0x%08X", GetQId(), indexPtr, se.d.dw7);
    LOG_NRM("SQ %d, SE %d, DWORD8:  0x%08X", GetQId(), indexPtr, se.d.dw8);
    LOG_NRM("SQ %d, SE %d, DWORD9:  0x%08X", GetQId(), indexPtr, se.d.dw9);
    LOG_NRM("SQ %d, SE %d, DWORD10: 0x%08X", GetQId(), indexPtr, se.d.dw10);
    LOG_NRM("SQ %d, SE %d, DWORD11: 0x%08X", GetQId(), indexPtr, se.d.dw11);
    LOG_NRM("SQ %d, SE %d, DWORD12: 0x%08X", GetQId(), indexPtr, se.d.dw12);
    LOG_NRM("SQ %d, SE %d, DWORD13: 0x%08X", GetQId(), indexPtr, se.d.dw13);
    LOG_NRM("SQ %d, SE %d, DWORD14: 0x%08X", GetQId(), indexPtr, se.d.dw14);
    LOG_NRM("SQ %d, SE %d, DWORD15: 0x%08X", GetQId(), indexPtr, se.d.dw15);
}


void
SQ::Send(SharedCmdPtr cmd)
{
    int rc;
    struct nvme_64b_send io;

    io.q_id = GetQId();
    io.bit_mask = (send_64b_bitmask)(cmd->GetPrpBitmask() | cmd->GetMetaBitmask());
    io.meta_buf_id = cmd->GetMetaBufferID();
    io.data_buf_size = cmd->GetPrpBufferSize();
    io.data_buf_ptr = cmd->GetROPrpBuffer();
    io.cmd_buf_ptr = cmd->GetCmd()->GetBuffer();
    io.cmd_set = cmd->GetCmdSet();
    io.data_dir = cmd->GetDataDir();

    LOG_NRM("Send cmd set %d, opcode 0x%02X, data buf size %d, to SQ id 0x%02X",
        io.cmd_set, cmd->GetOpcode(), io.data_buf_size, io.q_id);
    if ((rc = ioctl(mFd, NVME_IOCTL_SEND_64B_CMD, &io)) < 0) {
        LOG_ERR("Error sending cmd, rc =%d", rc);
        throw exception();
    }
}


void
SQ::Ring()
{
    int rc;
    uint16_t sqId = GetQId();

    LOG_NRM("Ring doorbell for SQ %d", sqId);
    if ((rc = ioctl(mFd, NVME_IOCTL_RING_SQ_DOORBELL, sqId)) < 0) {
        LOG_ERR("Error ringing doorbell, rc =%d", rc);
        throw exception();
    }
}
