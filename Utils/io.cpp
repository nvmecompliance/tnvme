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

#include "boost/format.hpp"
#include "kernelAPI.h"
#include "globals.h"
#include "io.h"


IO::IO()
{
}


IO::~IO()
{
}


void
IO::SendCmdToHdw(string grpName, string testName, uint16_t ms,
    SharedSQPtr sq, SharedCQPtr cq, SharedCmdPtr cmd, string qualify,
    bool verbose)
{
    uint16_t numCE;
    uint32_t isrCount;
    uint32_t isrCountB4;
    string work;


    if ((numCE = cq->ReapInquiry(isrCountB4, true)) != 0) {
        throw FrmwkEx("Require 0 CE's within CQ %d, not upheld, found %d",
            cq->GetQId(), numCE);
    }

    LOG_NRM("Send the cmd to hdw via SQ %d", sq->GetQId());
    sq->Send(cmd);
    if (verbose) {
        work = str(boost::format(
            "Just B4 ringing SQ %d doorbell, dump entire SQ") % sq->GetQId());
        sq->Dump(FileSystem::PrepLogFile(grpName, testName,
            "sq." + cmd->GetName(), qualify), work);
    }
    sq->Ring();


    LOG_NRM("Wait for the CE to arrive in CQ %d", cq->GetQId());
    if (cq->ReapInquiryWaitSpecify(ms, 1, numCE, isrCount) == false) {
        work = str(boost::format(
            "Unable to see any CE's in CQ %d, dump entire CQ") % cq->GetQId());
        cq->Dump(
            FileSystem::PrepLogFile(grpName, testName, "cq." + cmd->GetName(),
            qualify), work);
        throw FrmwkEx(work);
    } else if (numCE != 1) {
        throw FrmwkEx("1 cmd caused %d CE's to arrive in CQ %d",
            numCE, cq->GetQId());
    }
    if (verbose) {
        work = str(boost::format("Just B4 reaping CQ %d, dump entire CQ") %
            cq->GetQId());
        cq->Dump(FileSystem::PrepLogFile(grpName, testName,
            "cq." + cmd->GetName(), qualify), work);
    }

    // throws if an error occurs
    ReapCE(cq, numCE, isrCount, grpName, testName, qualify);

    // Single cmd submitted on empty ASQ should always yield 1 IRQ on ACQ
    if (gCtrlrConfig->IrqsEnabled() && cq->GetIrqEnabled() &&
        (isrCount != (isrCountB4 + 1))) {

        throw FrmwkEx("CQ using IRQ's, but IRQ count not expected (%d != %d)",
            isrCount, (isrCountB4 + 1));
    }

    if (verbose) {
        cmd->Dump(FileSystem::PrepLogFile(grpName, testName,
            cmd->GetName(), qualify), "A cmd's contents dumped");
    }
}


void
IO::ReapCE(SharedCQPtr cq, uint16_t numCE, uint32_t &isrCount,
    string grpName, string testName, string qualify, CEStat status)
{
    uint16_t ceRemain;
    uint16_t numReaped;
    string work;


    LOG_NRM("The CQ's metrics before reaping holds head_ptr");
    struct nvme_gen_cq cqMetrics = cq->GetQMetrics();
    KernelAPI::LogCQMetrics(cqMetrics);

    LOG_NRM("Reaping CE from CQ %d, requires memory to hold CE", cq->GetQId());
    SharedMemBufferPtr ceMem = SharedMemBufferPtr(new MemBuffer());
    if ((numReaped = cq->Reap(ceRemain, ceMem, isrCount, numCE, true)) != 1) {
        work = str(boost::format("Verified CE's exist, desired %d, reaped %d")
            % numCE % numReaped);
        cq->Dump(
            FileSystem::PrepLogFile(grpName, testName, "cq.error", qualify),
            work);
        throw FrmwkEx(work);
    }
    union CE ce = cq->PeekCE(cqMetrics.head_ptr);
    ProcessCE::Validate(ce, status);  // throws upon error
}

