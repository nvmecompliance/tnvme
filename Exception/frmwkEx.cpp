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

#include <stdio.h>
#include <stdarg.h>
#include "frmwkEx.h"
#include "tnvme.h"
#include "globals.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/io.h"
#include "../Cmds/getLogPage.h"

#define GRP_NAME            "post"
#define TEST_NAME           "failure"

bool FrmwkEx::mPrelimProcessingInProgress = false;


FrmwkEx::FrmwkEx(string filename, int lineNum)
{
    LOG_ERR("Exception: %s:#%d: FAILURE: no reason supplied",
        filename.c_str(), lineNum);
    DumpStateOfTheSystem();
}


FrmwkEx::FrmwkEx(string filename, int lineNum, string &msg)
{
    mMsg = msg;
    LOG_ERR("Exception: %s:#%d: FAILURE: %s", filename.c_str(), lineNum,
        mMsg.c_str());
    DumpStateOfTheSystem();
}


FrmwkEx::FrmwkEx(string filename, int lineNum, const char *fmt, ...)
{
    char work[256];
    va_list arg;

    va_start(arg, fmt);
    vsnprintf(work, sizeof(work), fmt, arg);
    va_end(arg);

    mMsg = work;
    LOG_ERR("Exception: %s:#%d: FAILURE: %s", filename.c_str(), lineNum,
        mMsg.c_str());
    DumpStateOfTheSystem();
}


FrmwkEx::~FrmwkEx()
{
    // Reset for possible subsequent exception to occur, i.e. ignore errors
    mPrelimProcessingInProgress = false;
}

void
FrmwkEx::DumpStateOfTheSystem()
{
    // Mark this point in /var/log/messages from dnvme's logging output
    KernelAPI::WriteToDnvmeLog("-------START POST FAILURE STATE DUMP-------");
    LOG_NRM("-------------------------------------------");
    LOG_NRM("-------START POST FAILURE STATE DUMP-------");

    // Avoid exception processing within exception processing looping
    if (mPrelimProcessingInProgress) {
        LOG_NRM("Detected exception within an exception");
        return;
    }
    mPrelimProcessingInProgress = true;

    // Only do post failure extraction and dumping if authorized to do so
    if (gCmdLine.postfail == false) {
        // Allways reset to sync kernel with user space or risk core dump
        if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
            throw FrmwkEx(HERE, "Exception handler()");
    } else {
        PreliminaryProcessing();   // Override in children provides custom logic
        gCtrlrConfig->SetState(ST_DISABLE);
    }
    LOG_NRM("--------END POST FAILURE STATE DUMP--------");
    LOG_NRM("-------------------------------------------");
}


void
FrmwkEx::PreliminaryProcessing()
{
    // First gather all non-intrusive data, things that won't change the
    // state of the DUT, effectively taking a snapshot.
    KernelAPI::DumpKernelMetrics(FileSystem::PrepDumpFile(GRP_NAME,
        TEST_NAME, "kmetrics"));
    KernelAPI::DumpPciSpaceRegs(FileSystem::PrepDumpFile(GRP_NAME,
        TEST_NAME, "pci", "regs"), false);
    KernelAPI::DumpCtrlrSpaceRegs(FileSystem::PrepDumpFile(GRP_NAME,
        TEST_NAME, "ctrl", "regs"), false);

    // Now we can change the state of the DUT. We don't know the state of
    // the ACQ/ASQ, or even if there are any in existence; place the DUT
    // into a well known state and then interact gathering instrusive data
    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE, "Exception handler()");

    SharedACQPtr acq = SharedACQPtr(new ACQ(gDutFd));
    acq->Init(2);

    SharedASQPtr asq = SharedASQPtr(new ASQ(gDutFd));
    asq->Init(2);

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE, "Exception handler()");

    LOG_NRM("Create get log page cmd and assoc some buffer memory");
    SharedGetLogPagePtr getLogPg = SharedGetLogPagePtr(new GetLogPage());
    getLogPg->SetLID(GetLogPage::LOGID_ERROR_INFO);

    ConstSharedIdentifyPtr idCmdCtrlr = gInformative->GetIdentifyCmdCtrlr();
    uint64_t errLogPgEntries = (idCmdCtrlr->GetValue(IDCTRLRCAP_ELPE) + 1);
    uint32_t bufSize = (errLogPgEntries * GetLogPage::ERRINFO_DATA_SIZE);
    uint16_t numDWAvail = (bufSize / sizeof(uint32_t));
    getLogPg->SetNUMD(numDWAvail);

    SharedMemBufferPtr cmdMem = SharedMemBufferPtr(new MemBuffer());
    cmdMem->InitAlignment(bufSize, sysconf(_SC_PAGESIZE), true, 0);
    send_64b_bitmask prpReq =
        (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    getLogPg->SetPrpBuffer(prpReq, cmdMem);

    IO::SendAndReapCmd(GRP_NAME, TEST_NAME, CALC_TIMEOUT_ms(1), asq, acq,
        getLogPg, "", false);
    getLogPg->Dump(FileSystem::PrepDumpFile(GRP_NAME, TEST_NAME,
        "LogPageErr"), "Failed test post dump of error log page:");
}

