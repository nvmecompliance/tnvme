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
#include "attributes_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/io.h"
#include "../Cmds/datasetMgmt.h"

#define     MAX_RANGES      256

namespace GrpNVMDatasetMgmtCmd {


Attributes_r10b::Attributes_r10b(int fd, string mGrpName,
    string mTestName, ErrorRegs errRegs) :
    Test(fd, mGrpName, mTestName, SPECREV_10b, errRegs)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Verify all combinations of attributes for all ranges");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "For namspc = 1; create a single buffer where offset into 1st "
        "page = 0, size = 4KB; initialize this buffer describing 256 "
        "ranges equally divided among the total address space spec'd "
        "by Identify.NCAP. Issue the dataset mgmt cmd multiple times "
        "iterating through all permutations of all context attributes "
        "except the reserved values, expect success. Testing to verify "
        "all legal attributes are accepted.");
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


void
Attributes_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */
    string work;

    ConstSharedIdentifyPtr idCtrlrStruct = gInformative->GetIdentifyCmdCtrlr();
    uint32_t nn = (uint32_t)idCtrlrStruct->GetValue(IDCTRLRCAP_NN);
    if (nn == 0) {
        LOG_WARN("Required to support atleast 1 namspc to run this test.");
        return;
    }

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
    RangeDef *rangePtr = (RangeDef *)rangeMem->GetBuffer();
    for (uint32_t numRange = 0; numRange < MAX_RANGES; numRange++) {
        rangePtr->slba = 0;
        rangePtr->length = (ncap / MAX_RANGES);
        if (((ncap % MAX_RANGES) != 0) && (numRange == (MAX_RANGES - 1)))
            rangePtr->length = (ncap - ((MAX_RANGES - 1) * rangePtr->length));

        for (list <uint32_t>::iterator ctxAttrib = ctrAttribs.begin();
            ctxAttrib != ctrAttribs.end(); ctxAttrib++) {
            LOG_NRM("Range #%d, (LenLBAs, SLBA, CtxAttrib) = (%d, %ld, 0x%X)",
                numRange, rangePtr->length, rangePtr->slba, *ctxAttrib);

            memcpy(&rangePtr->ctxAttrib, &(*ctxAttrib),
                sizeof(struct CtxAttrib));
            work = str(boost::format("numRange%d.ctxAttrib0x%X ") %
                numRange % *ctxAttrib);

            bool enableLog = false;
            if ((((numRange < 8) == 0) || ((numRange - 8) < MAX_RANGES)) == 0)
                enableLog = true;

            IO::SendAndReapCmd(mGrpName, mTestName, DEFAULT_CMD_WAIT_ms,
                iosq, iocq, datasetMgmtCmd, work, enableLog);
        }
        rangePtr++;
    }
}


list<uint32_t>
Attributes_r10b::GetUniqueCtxAttribs()
{
    list <uint32_t> ctrAttribs;

    uint32_t ctxVal;
    struct CtxAttrib ctxAttrib;
    for (uint64_t  iPow2 = 1; iPow2 <= UINT32_MAX; iPow2 <<= 1) {
        for (uint64_t j = (iPow2 - 1); j <= (iPow2 + 1); j++) {
            if (j > UINT32_MAX)
                break;
            ctxVal = j;
            memcpy(&ctxAttrib, &ctxVal, sizeof(struct CtxAttrib));
            // skip reserved values.
            ctxAttrib.reserved0 = 0;
            ctxAttrib.reserved1 = 0;
            ctxAttrib.AF = ((ctxAttrib.AF & 7) > 5) ? 5 : (ctxAttrib.AF & 7);
            memcpy(&ctxVal, &ctxAttrib, sizeof(uint32_t));
            ctrAttribs.remove(ctxVal); // remove duplicates.
            ctrAttribs.push_back(ctxVal);
        }
    }
    return ctrAttribs;
}

}   // namespace

