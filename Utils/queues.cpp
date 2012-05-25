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

#include "queues.h"
#include "globals.h"
#include "io.h"
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


SharedIOCQPtr
Queues::CreateIOCQContigToHdw(string grpName, string testName,
    uint16_t ms, SharedASQPtr asq, SharedACQPtr acq, uint16_t qId,
    uint32_t numEntries, bool grpLifetime, string grpID, bool irqEnabled,
    uint16_t irqVec, string qualify, bool verbose)
{
    char work[20];
    string workStr;
    SharedIOCQPtr iocq;


    // Effectively make the qualifier an extension to a filename
    if (qualify.length() == 0)
        snprintf(work, sizeof(work), "QID%d", qId);
    else
        snprintf(work, sizeof(work), "QID%d.%s", qId, qualify.c_str());
    workStr = work;

    if (grpLifetime) {
        iocq = CAST_TO_IOCQ(gRsrcMngr->AllocObj(Trackable::OBJ_IOCQ, grpID));
    } else {
        LOG_NRM("Create an IOCQ object with test lifetime");
        iocq = SharedIOCQPtr(new IOCQ(gDutFd));
    }
    LOG_NRM("Allocate contiguous memory; IOCQ has ID=%d", qId);
    iocq->Init(qId, numEntries, irqEnabled, irqVec);

    LOG_NRM("Form a Create IOCQ cmd to perform queue creation");
    SharedCreateIOCQPtr createIOCQCmd = SharedCreateIOCQPtr(new CreateIOCQ());
    createIOCQCmd->Init(iocq);

    IO::SendAndReapCmd(grpName, testName, ms, asq, acq, createIOCQCmd, workStr,
        verbose);
    return iocq;
}


SharedIOSQPtr
Queues::CreateIOSQContigToHdw(string grpName, string testName,
    uint16_t ms, SharedASQPtr asq, SharedACQPtr acq, uint16_t qId,
    uint32_t numEntries, bool grpLifetime, string grpID, uint16_t cqId,
    uint8_t priority, string qualify, bool verbose)
{
    char work[20];
    string workStr;
    SharedIOSQPtr iosq;


    // Effectively make the qualifier an extension to a filename
    if (qualify.length() == 0)
        snprintf(work, sizeof(work), "QID%d", qId);
    else
        snprintf(work, sizeof(work), "QID%d.%s", qId, qualify.c_str());
    workStr = work;

    if (grpLifetime) {
        iosq = CAST_TO_IOSQ(gRsrcMngr->AllocObj(Trackable::OBJ_IOSQ, grpID));
    } else {
        LOG_NRM("Create an IOSQ object with test lifetime");
        iosq = SharedIOSQPtr(new IOSQ(gDutFd));
    }
    LOG_NRM("Allocate contiguous memory; IOSQ has ID=%d", qId);
    iosq->Init(qId, numEntries, cqId, priority);

    LOG_NRM("Form a Create IOSQ cmd to perform queue creation");
    SharedCreateIOSQPtr createIOSQCmd = SharedCreateIOSQPtr(new CreateIOSQ());
    createIOSQCmd->Init(iosq);

    IO::SendAndReapCmd(grpName, testName, ms, asq, acq, createIOSQCmd, workStr,
        verbose);
    return iosq;
}


SharedIOCQPtr
Queues::CreateIOCQDiscontigToHdw(string grpName, string testName,
    uint16_t ms, SharedASQPtr asq, SharedACQPtr acq, uint16_t qId,
    uint32_t numEntries, bool grpLifetime, string grpID, bool irqEnabled,
    uint16_t irqVec, SharedMemBufferPtr qBackedMem, string qualify,
    bool verbose)
{
    uint8_t ioqes;
    char work[20];
    string workStr;
    SharedIOCQPtr iocq;


    // Effectively make the qualifier an extension to a filename
    if (qualify.length() == 0)
        snprintf(work, sizeof(work), "QID%d", qId);
    else
        snprintf(work, sizeof(work), "QID%d.%s", qId, qualify.c_str());
    workStr = work;

    // Validate the memory correlates to the other params passed in
    if (gCtrlrConfig->GetIOCQES(ioqes) == false)
        throw FrmwkEx(HERE);
    if (qBackedMem->GetBufSize() < (uint32_t)((1 << ioqes) * numEntries)) {
        throw FrmwkEx(HERE,
            "Supplied buffer size = 0x%08X; inconsistent w/ creation",
            ((1 << ioqes) * numEntries));
    }

    if (grpLifetime) {
        iocq = CAST_TO_IOCQ(gRsrcMngr->AllocObj(Trackable::OBJ_IOCQ, grpID));
    } else {
        LOG_NRM("Create an IOCQ object with test lifetime");
        iocq = SharedIOCQPtr(new IOCQ(gDutFd));
    }

    LOG_NRM("Assoc discontiguous memory; IOCQ has ID=%d", qId);
    iocq->Init(qId, numEntries, qBackedMem, irqEnabled, irqVec);

    LOG_NRM("Form a Create IOCQ cmd to perform queue creation");
    SharedCreateIOCQPtr createIOCQCmd = SharedCreateIOCQPtr(new CreateIOCQ());
    createIOCQCmd->Init(iocq);

    IO::SendAndReapCmd(grpName, testName, ms, asq, acq, createIOCQCmd, workStr,
        verbose);
    return iocq;
}


SharedIOSQPtr
Queues::CreateIOSQDiscontigToHdw(string grpName, string testName,
    uint16_t ms, SharedASQPtr asq, SharedACQPtr acq, uint16_t qId,
    uint32_t numEntries, bool grpLifetime, string grpID, uint16_t cqId,
    uint8_t priority, SharedMemBufferPtr qBackedMem, string qualify,
    bool verbose)
{
    uint8_t ioqes;
    char work[20];
    string workStr;
    SharedIOSQPtr iosq;


    // Effectively make the qualifier an extension to a filename
    if (qualify.length() == 0)
        snprintf(work, sizeof(work), "QID%d", qId);
    else
        snprintf(work, sizeof(work), "QID%d.%s", qId, qualify.c_str());
    workStr = work;

    // Validate the memory correlates to the other params passed in
    if (gCtrlrConfig->GetIOCQES(ioqes) == false)
        throw FrmwkEx(HERE);
    if (qBackedMem->GetBufSize() < (uint32_t)((1 << ioqes) * numEntries)) {
        throw FrmwkEx(HERE,
            "Supplied buffer size = 0x%08X; inconsistent w/ creation",
            ((1 << ioqes) * numEntries));
    }

    if (grpLifetime) {
        iosq = CAST_TO_IOSQ(gRsrcMngr->AllocObj(Trackable::OBJ_IOSQ, grpID));
    } else {
        LOG_NRM("Create an IOSQ object with test lifetime");
        iosq = SharedIOSQPtr(new IOSQ(gDutFd));
    }
    LOG_NRM("Assoc discontiguous memory; IOSQ has ID=%d", qId);
    iosq->Init(qId, numEntries, qBackedMem, cqId, priority);

    LOG_NRM("Form a Create IOSQ cmd to perform queue creation");
    SharedCreateIOSQPtr createIOSQCmd = SharedCreateIOSQPtr(new CreateIOSQ());
    createIOSQCmd->Init(iosq);

    IO::SendAndReapCmd(grpName, testName, ms, asq, acq, createIOSQCmd, workStr,
        verbose);
    return iosq;
}


void
Queues::DeleteIOCQToHdw(string grpName, string testName, uint16_t ms,
    SharedIOCQPtr iocq, SharedASQPtr asq, SharedACQPtr acq, string qualify,
    bool verbose)
{
    char work[20];
    string workStr;


    // Effectively make the qualifier an extension to a filename
    if (qualify.length() == 0) {
        snprintf(work, sizeof(work), "QID%d", iocq->GetQId());
    } else {
        snprintf(work, sizeof(work), "QID%d.%s", iocq->GetQId(),
            qualify.c_str());
    }
    workStr = work;

    LOG_NRM("Form a Delete IOCQ cmd to perform queue deletion");
    SharedDeleteIOCQPtr deleteIOCQCmd = SharedDeleteIOCQPtr(new DeleteIOCQ());
    deleteIOCQCmd->Init(iocq);

    IO::SendAndReapCmd(grpName, testName, ms, asq, acq, deleteIOCQCmd, workStr,
        verbose);
}


void
Queues::DeleteIOSQToHdw(string grpName, string testName, uint16_t ms,
    SharedIOSQPtr iosq, SharedASQPtr asq, SharedACQPtr acq, string qualify,
    bool verbose)
{
    char work[20];
    string workStr;


    // Effectively make the qualifier an extension to a filename
    if (qualify.length() == 0) {
        snprintf(work, sizeof(work), "QID%d", iosq->GetQId());
    } else {
        snprintf(work, sizeof(work), "QID%d.%s", iosq->GetQId(),
            qualify.c_str());
    }
    workStr = work;

    LOG_NRM("Form a Delete IOSQ cmd to perform queue deletion");
    SharedDeleteIOSQPtr deleteIOSQCmd = SharedDeleteIOSQPtr(new DeleteIOSQ());
    deleteIOSQCmd->Init(iosq);

    IO::SendAndReapCmd(grpName, testName, ms, asq, acq, deleteIOSQCmd, workStr,
        verbose);
}


bool
Queues::SupportDiscontigIOQ()
{
    uint64_t regVal;
    if (gRegisters->Read(CTLSPC_CAP, regVal) == false)
        throw FrmwkEx(HERE, "Failed reading ctrlr capabilities (CAP) register");

    if (regVal & CAP_CQR)
        return false;

    return true;
}
