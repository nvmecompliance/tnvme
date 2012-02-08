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

#include <math.h>
#include "queues.h"
#include "kernelAPI.h"
#include "globals.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Cmds/deleteIOCQ.h"
#include "../Cmds/deleteIOSQ.h"


Queues::Queues()
{
}


Queues::~Queues()
{
}


SharedCreateIOCQPtr
Queues::CreateIOCQContig(int fd, string grpName, string testName,
    uint16_t ms, SharedASQPtr asq, SharedACQPtr acq, uint16_t qId,
    uint16_t numEntries, bool grpLifetime, string grpID, bool irqEnabled,
    uint16_t irqVec)
{
    char work[20];
    string qualify;
    SharedIOCQPtr iocq;


    // Effectively make the qualifier an extension to a filename
    snprintf(work, sizeof(work), ".QID.%d", qId);
    qualify = work;

    if (grpLifetime) {
        LOG_NRM("Create an IOCQ object with group lifetime");
        iocq = CAST_TO_IOCQ(gRsrcMngr->AllocObj(Trackable::OBJ_IOCQ, grpID));
    } else {
        LOG_NRM("Create an IOCQ object with test lifetime");
        iocq = SharedIOCQPtr(new IOCQ(fd));
    }
    LOG_NRM("Allocate contiguous memory, ID=%d for the IOCQ", qId);
    iocq->Init(qId, numEntries, irqEnabled, irqVec);

    return InvokeIOCQInHdw(fd, grpName, testName, ms, asq, acq, iocq, qualify);
}


SharedCreateIOSQPtr
Queues::CreateIOSQContig(int fd, string grpName, string testName,
    uint16_t ms, SharedASQPtr asq, SharedACQPtr acq, uint16_t qId,
    uint16_t numEntries, bool grpLifetime, string grpID, uint16_t cqId,
    uint8_t priority)
{
    char work[20];
    string qualify;
    SharedIOSQPtr iosq;


    // Effectively make the qualifier an extension to a filename
    snprintf(work, sizeof(work), ".QID.%d", qId);
    qualify = work;

    if (grpLifetime) {
        LOG_NRM("Create an IOSQ object with group lifetime");
        iosq = CAST_TO_IOSQ(gRsrcMngr->AllocObj(Trackable::OBJ_IOSQ, grpID));
    } else {
        LOG_NRM("Create an IOSQ object with test lifetime");
        iosq = SharedIOSQPtr(new IOSQ(fd));
    }
    LOG_NRM("Allocate contiguous memory, ID=%d for the IOSQ", qId);
    iosq->Init(qId, numEntries, cqId, priority);

    return InvokeIOSQInHdw(fd, grpName, testName, ms, asq, acq, iosq, qualify);
}


SharedCreateIOCQPtr
Queues::CreateIOCQDiscontig(int fd, string grpName, string testName,
    uint16_t ms, SharedASQPtr asq, SharedACQPtr acq, uint16_t qId,
    uint16_t numEntries, bool grpLifetime, string grpID, bool irqEnabled,
    uint16_t irqVec, SharedMemBufferPtr qBackedMem)
{
    uint8_t ioqes;
    char work[20];
    string qualify;
    SharedIOCQPtr iocq;


    // Effectively make the qualifier an extension to a filename
    snprintf(work, sizeof(work), ".QID.%d", qId);
    qualify = work;

    // Validate the memory correlates to the other params passed in
    if (gCtrlrConfig->GetIOCQES(ioqes) == false)
        throw exception();
    if (qBackedMem->GetBufSize() < (uint32_t)pow(2, ioqes)) {
        LOG_ERR("Supplied buffer size = 0x%08X; inconsistent w/ creation",
            (uint32_t)pow(2, ioqes));
        throw exception();
    }

    if (grpLifetime) {
        LOG_NRM("Create an IOCQ object with group lifetime");
        iocq = CAST_TO_IOCQ(gRsrcMngr->AllocObj(Trackable::OBJ_IOCQ, grpID));
    } else {
        LOG_NRM("Create an IOCQ object with test lifetime");
        iocq = SharedIOCQPtr(new IOCQ(fd));
    }

    LOG_NRM("Assoc discontiguous memory, ID=%d for the IOCQ", qId);
    iocq->Init(qId, numEntries, qBackedMem, irqEnabled, irqVec);

    return InvokeIOCQInHdw(fd, grpName, testName, ms, asq, acq, iocq, qualify);
}


SharedCreateIOSQPtr
Queues::CreateIOSQDiscontig(int fd, string grpName, string testName,
    uint16_t ms, SharedASQPtr asq, SharedACQPtr acq, uint16_t qId,
    uint16_t numEntries, bool grpLifetime, string grpID, uint16_t cqId,
    uint8_t priority, SharedMemBufferPtr qBackedMem)
{
    uint8_t ioqes;
    char work[20];
    string qualify;
    SharedIOSQPtr iosq;


    // Effectively make the qualifier an extension to a filename
    snprintf(work, sizeof(work), ".QID.%d", qId);
    qualify = work;

    // Validate the memory correlates to the other params passed in
    if (gCtrlrConfig->GetIOCQES(ioqes) == false)
        throw exception();
    if (qBackedMem->GetBufSize() < (uint32_t)pow(2, ioqes)) {
        LOG_ERR("Supplied buffer size = 0x%08X; inconsistent w/ creation",
            (uint32_t)pow(2, ioqes));
        throw exception();
    }

    if (grpLifetime) {
        LOG_NRM("Create an IOSQ object with group lifetime");
        iosq = CAST_TO_IOSQ(gRsrcMngr->AllocObj(Trackable::OBJ_IOSQ, grpID));
    } else {
        LOG_NRM("Create an IOSQ object with test lifetime");
        iosq = SharedIOSQPtr(new IOSQ(fd));
    }
    LOG_NRM("Assoc discontiguous memory, ID=%d for the IOSQ", qId);
    iosq->Init(qId, numEntries, qBackedMem, cqId, priority);

    return InvokeIOSQInHdw(fd, grpName, testName, ms, asq, acq, iosq, qualify);
}


SharedCreateIOCQPtr
Queues::InvokeIOCQInHdw(int fd, string grpName, string testName,
    uint16_t ms, SharedASQPtr asq, SharedACQPtr acq, SharedIOCQPtr iocq,
    string qualify)
{
    uint16_t numCE;

    LOG_NRM("Form a Create IOCQ cmd to perform queue creation");
    SharedCreateIOCQPtr createIOCQCmd = SharedCreateIOCQPtr(new CreateIOCQ(fd));
    createIOCQCmd->Init(iocq);


    LOG_NRM("Send the Create IOCQ cmd to hdw");
    asq->Send(createIOCQCmd);
    asq->Dump(FileSystem::PrepLogFile(grpName, testName, "asq",
        ("createIOCQCmd" + qualify)),
        "Just B4 ringing SQ0 doorbell, dump entire SQ");
    asq->Ring();


    LOG_NRM("Wait for the CE to arrive in ACQ");
    if (acq->ReapInquiryWaitSpecify(ms, 1, numCE) == false) {
        LOG_ERR("Unable to see CE of Create IOCQ cmd");
        acq->Dump(
            FileSystem::PrepLogFile(grpName, testName, "acq",
            ("createIOCQCmd" + qualify)),
            "Unable to see any CE's in CQ0, dump entire CQ");
        throw exception();
    } else if (numCE != 1) {
        LOG_ERR("The ACQ should only have 1 CE as a result of cmd");
        throw exception();
    }
    acq->Dump(FileSystem::PrepLogFile(grpName, testName, "acq",
        ("createIOCQCmd" + qualify)), "Just B4 reaping CQ0, dump entire CQ");


    ReapCE(acq, numCE);    // throws if an error occurs
    createIOCQCmd->Dump(
        FileSystem::PrepLogFile(grpName, testName, ("IOCQ" + qualify)),
        "The complete IOCQ contents, not yet used, but is created.");
    return createIOCQCmd;
}


SharedCreateIOSQPtr
Queues::InvokeIOSQInHdw(int fd, string grpName, string testName,
    uint16_t ms, SharedASQPtr asq, SharedACQPtr acq, SharedIOSQPtr iosq,
    string qualify)
{
    uint16_t numCE;

    LOG_NRM("Form a Create IOSQ cmd to perform queue creation");
    SharedCreateIOSQPtr createIOSQCmd = SharedCreateIOSQPtr(new CreateIOSQ(fd));
    createIOSQCmd->Init(iosq);


    LOG_NRM("Send the Create IOSQ cmd to hdw");
    asq->Send(createIOSQCmd);
    asq->Dump(FileSystem::PrepLogFile(grpName, testName, "asq",
        ("createIOSQCmd" + qualify)),
        "Just B4 ringing SQ0 doorbell, dump entire SQ");
    asq->Ring();


    LOG_NRM("Wait for the CE to arrive in ACQ");
    if (acq->ReapInquiryWaitSpecify(ms, 1, numCE) == false) {
        LOG_ERR("Unable to see CE of Create IOSQ cmd");
        acq->Dump(
            FileSystem::PrepLogFile(grpName, testName, "acq",
            ("createIOSQCmd" + qualify)),
            "Unable to see any CE's in CQ0, dump entire CQ");
        throw exception();
    } else if (numCE != 1) {
        LOG_ERR("The ACQ should only have 1 CE as a result of cmd");
        throw exception();
    }
    acq->Dump(FileSystem::PrepLogFile(grpName, testName, "acq",
        ("createIOSQCmd" + qualify)), "Just B4 reaping CQ0, dump entire CQ");


    ReapCE(acq, numCE);    // throws if an error occurs
    createIOSQCmd->Dump(
        FileSystem::PrepLogFile(grpName, testName, ("IOSQ" + qualify)),
        "The complete IOSQ contents, not yet used, but is created.");
    return createIOSQCmd;
}


void
Queues::DeleteIOCQInHdw(int fd, string grpName, string testName, uint16_t ms,
    SharedIOCQPtr iocq, SharedASQPtr asq, SharedACQPtr acq)
{
    uint16_t numCE;
    char work[20];
    string qualify;


    // Effectively make the qualifier an extension to a filename
    snprintf(work, sizeof(work), ".QID.%d", iocq->GetQId());
    qualify = work;

    LOG_NRM("Form a Delete IOCQ cmd to perform queue deletion");
    SharedDeleteIOCQPtr deleteIOCQCmd = SharedDeleteIOCQPtr(new DeleteIOCQ(fd));
    deleteIOCQCmd->Init(iocq);


    LOG_NRM("Send the Delete IOCQ cmd to hdw");
    asq->Send(deleteIOCQCmd);
    asq->Dump(FileSystem::PrepLogFile(grpName, testName, "asq",
        ("deleteIOCQCmd" + qualify)),
        "Just B4 ringing SQ0 doorbell, dump entire SQ");
    asq->Ring();


    LOG_NRM("Wait for the CE to arrive in ACQ");
    if (acq->ReapInquiryWaitSpecify(ms, 1, numCE) == false) {
        LOG_ERR("Unable to see CE of Delete IOCQ cmd");
        acq->Dump(
            FileSystem::PrepLogFile(grpName, testName, "acq",
            ("deleteIOCQCmd" + qualify)),
            "Unable to see any CE's in CQ0, dump entire CQ");
        throw exception();
    }
    acq->Dump(FileSystem::PrepLogFile(grpName, testName, "acq",
        ("deleteIOCQCmd" + qualify)), "Just B4 reaping CQ0, dump entire CQ");

    ReapCE(acq, numCE);    // throws if an error occurs
}


void
Queues::DeleteIOSQInHdw(int fd, string grpName, string testName, uint16_t ms,
    SharedIOSQPtr iosq, SharedASQPtr asq, SharedACQPtr acq)
{
    uint16_t numCE;
    char work[20];
    string qualify;


    // Effectively make the qualifier an extension to a filename
    snprintf(work, sizeof(work), ".QID.%d", iosq->GetQId());
    qualify = work;

    LOG_NRM("Form a Delete IOSQ cmd to perform queue deletion");
    SharedDeleteIOSQPtr deleteIOSQCmd = SharedDeleteIOSQPtr(new DeleteIOSQ(fd));
    deleteIOSQCmd->Init(iosq);


    LOG_NRM("Send the Delete IOSQ cmd to hdw");
    asq->Send(deleteIOSQCmd);
    asq->Dump(FileSystem::PrepLogFile(grpName, testName, "asq",
        ("deleteIOSQCmd" + qualify)),
        "Just B4 ringing SQ0 doorbell, dump entire SQ");
    asq->Ring();


    LOG_NRM("Wait for the CE to arrive in ACQ");
    if (acq->ReapInquiryWaitSpecify(ms, 1, numCE) == false) {
        LOG_ERR("Unable to see CE of Delete IOSQ cmd");
        acq->Dump(
            FileSystem::PrepLogFile(grpName, testName, "acq",
            ("deleteIOSQCmd" + qualify)),
            "Unable to see any CE's in CQ0, dump entire CQ");
        throw exception();
    }
    acq->Dump(FileSystem::PrepLogFile(grpName, testName, "acq",
        ("deleteIOSQCmd" + qualify)), "Just B4 reaping CQ0, dump entire CQ");

    ReapCE(acq, numCE);    // throws if an error occurs
}


void
Queues::ReapCE(SharedACQPtr acq, uint16_t numCE)
{
    uint16_t ceRemain;
    uint16_t numReaped;


    LOG_NRM("The CQ's metrics before reaping holds head_ptr");
    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
    KernelAPI::LogCQMetrics(acqMetrics);

    LOG_NRM("Reaping CE from ACQ, requires memory to hold CE");
    SharedMemBufferPtr ceMem = SharedMemBufferPtr(new MemBuffer());
    if ((numReaped = acq->Reap(ceRemain, ceMem, numCE, true)) != 1) {
        LOG_ERR("Verified 1 CE, but reaping returned %d", numReaped);
        throw exception();
    }
    LOG_NRM("The reaped CE is...");
    acq->LogCE(acqMetrics.head_ptr);

    union CE ce = acq->PeekCE(acqMetrics.head_ptr);
    ProcessCE::ValidateStatus(ce);  // throws upon error
}
