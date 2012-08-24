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

#include <boost/format.hpp>
#include <string.h>
#include <assert.h>
#include "attributes_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/io.h"
#include "../Cmds/datasetMgmt.h"

#define MAX_RANGES      256

namespace GrpNVMDatasetMgmtCmd {


Attributes_r10b::Attributes_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Verify all combinations of attributes for all ranges");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Search for 1 of the following namspcs to run test. Find 1st bare "
        "namspc, or find 1st meta namspc, or find 1st E2E namspc. "
        "Create a single buffer where offset into 1st page = 0, size = 4KB; "
        "initialize this buffer describing 256 ranges equally divided among "
        "the total address space spec'd by Identify.NCAP. Issue the dataset "
        "mgmt cmd multiple times iterating through all permutations of all "
        "context attributes except the reserved values, expect success. "
        "Testing to verify all legal attributes are accepted.");
}


Attributes_r10b::~Attributes_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


Attributes_r10b::
Attributes_r10b(const Attributes_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


Attributes_r10b &
Attributes_r10b::operator=(const Attributes_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
Attributes_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    ConstSharedIdentifyPtr idCtrlrCap = gInformative->GetIdentifyCmdCtrlr();
    uint64_t oncs = idCtrlrCap->GetValue(IDCTRLRCAP_ONCS);
    if ((oncs & ONCS_SUP_DSM_CMD) == 0)
        return RUN_FALSE;

    preserve = preserve;    // Suppress compiler error/warning
    return RUN_TRUE;        // This test is never destructive
}


void
Attributes_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */
    string work;

    LOG_NRM("Lookup IOQS which were created in a prior test within group");
    SharedIOSQPtr iosq = CAST_TO_IOSQ(gRsrcMngr->GetObj(IOSQ_GROUP_ID));
    SharedIOCQPtr iocq = CAST_TO_IOCQ(gRsrcMngr->GetObj(IOCQ_GROUP_ID));

    LOG_NRM("Gather 1st namespace information.");
    Informative::Namspc namspcData = gInformative->Get1stBareMetaE2E();
    uint64_t ncap = namspcData.idCmdNamspc->GetValue(IDNAMESPC_NCAP);

    LOG_NRM("For namespace ID #%d; NCAP = 0x%08lX", namspcData.id, ncap);
    send_64b_bitmask prpBitmask =
            (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);

    LOG_NRM("Create dataset mgmt cmd to be used subsequently");
    SharedDatasetMgmtPtr datasetMgmtCmd =
        SharedDatasetMgmtPtr(new DatasetMgmt());

    LOG_NRM("Create memory to contain #%d dataset range defs", MAX_RANGES);
    SharedMemBufferPtr rangeMem = SharedMemBufferPtr(new MemBuffer());
    rangeMem->InitOffset1stPage((MAX_RANGES * sizeof(RangeDef)), 0, true);

    datasetMgmtCmd->SetNSID(namspcData.id);
    datasetMgmtCmd->SetNR(MAX_RANGES - 1);  // convert to 0-based
    datasetMgmtCmd->SetPrpBuffer(prpBitmask, rangeMem);

    list<uint32_t> ctrAttribs = GetUniqueCtxAttribs();
    for (list <uint32_t>::iterator ctxAttrib = ctrAttribs.begin();
        ctxAttrib != ctrAttribs.end(); ctxAttrib++) {
        LOG_NRM("Processing %d ranges for ctxAttrib = 0x%04X", MAX_RANGES,
            *ctxAttrib);

        RangeDef *rangePtr = (RangeDef *)rangeMem->GetBuffer();
        for (uint32_t numRange = 0; numRange < MAX_RANGES; numRange++) {
            rangePtr->slba = 0;
            rangePtr->length = (ncap / MAX_RANGES);
            if (numRange == (MAX_RANGES - 1)) // Last range consumes the rem.
                rangePtr->length += (ncap % MAX_RANGES);
            memcpy(&rangePtr->ctxAttrib, &(*ctxAttrib),
                sizeof(struct CtxAttrib));
            rangePtr++;
        }

        bool enableLog = false;
        if ((*ctxAttrib % 8) == 0)
            enableLog = true;

        work = str(boost::format("ctxAttrib0x%04X") % *ctxAttrib);
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq,
            datasetMgmtCmd, work, enableLog);
    }
}


list<uint32_t>
Attributes_r10b::GetUniqueCtxAttribs()
{
    list <uint32_t> ctrAttribs;
    uint32_t ctxVal;
    struct CtxAttrib *ctxAttrib = (struct CtxAttrib *)&ctxVal;
    assert(sizeof(uint32_t) == sizeof(struct CtxAttrib));

    // Set 10 bits of context attributes to different values, while
    // varying the CAS bits.
    for (uint32_t ctx = 1; ctx <= (1 << 10); ctx++) {
        ctxVal = ctx;
        // skip reserved values.
        if (ctxAttrib->reserved0 ||  ctxAttrib->reserved1 ||
            ((ctxAttrib->AF & 7) > 5)) {
            continue;
        }
        // Set command access size to different values
        ctxAttrib->CAS = (1 << (ctx % 8));
        ctrAttribs.push_back(ctxVal);
    }
    return ctrAttribs;
}

}   // namespace

