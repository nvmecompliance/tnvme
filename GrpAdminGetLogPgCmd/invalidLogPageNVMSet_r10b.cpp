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
#include "invalidLogPageNVMSet_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/io.h"
#include "../Cmds/getLogPage.h"

#define BUFFER_SIZE         0x4
#define MAX_LID             0xBF
#define NUMD                1

namespace GrpAdminGetLogPgCmd {


InvalidLogPageNVMSet_r10b::InvalidLogPageNVMSet_r10b
    (string grpName, string testName) : Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Issue GetLogPage for NVM cmd set, & cause SC=Invalid Log Page");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Set the NVM cmd set active. Create a buffer, size=4 to assoc with "
        "GetLogPage cmd, NUMD=1. Loop through all reserved LID values, "
        "expect failure. NOTE: for the NVM cmd set, there are no I/O cmd "
        "set specific LIDS and thus range[80h - BFh) is also reserved.");
}


InvalidLogPageNVMSet_r10b::~InvalidLogPageNVMSet_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


InvalidLogPageNVMSet_r10b::
InvalidLogPageNVMSet_r10b(const InvalidLogPageNVMSet_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


InvalidLogPageNVMSet_r10b &
InvalidLogPageNVMSet_r10b::operator=(const InvalidLogPageNVMSet_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
InvalidLogPageNVMSet_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    preserve = preserve;    // Suppress compiler error/warning
    return RUN_TRUE;        // This test is never destructive
}


void
InvalidLogPageNVMSet_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     *  \endverbatim
     */
    string work;

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    LOG_NRM("Create get log page cmd and assoc some buffer memory");
    SharedGetLogPagePtr getLogPgCmd = SharedGetLogPagePtr(new GetLogPage());
    SharedMemBufferPtr getLogPageMem = SharedMemBufferPtr(new MemBuffer());
    send_64b_bitmask prpReq =
            (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);

    getLogPageMem->InitOffset1stPage(BUFFER_SIZE, 0, true);
    getLogPgCmd->SetPrpBuffer(prpReq, getLogPageMem);
    getLogPgCmd->SetNUMD(NUMD - 1); // 0-based

    list<uint32_t> invalidLIDs = GetInvalidLIDs();
    for (list<uint32_t>::iterator invalidLID = invalidLIDs.begin();
        invalidLID != invalidLIDs.end(); invalidLID++) {
        LOG_NRM("Processing for invalid LID = 0x%04X", *invalidLID);
        getLogPgCmd->SetLID(*invalidLID);

        work = str(boost::format("invalidLID.%d") % *invalidLID);
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
            getLogPgCmd, work, true, CESTAT_INVAL_LOG_PAGE);
    }
}


list<uint32_t>
InvalidLogPageNVMSet_r10b::GetInvalidLIDs()
{
    list<uint32_t> invalidLIDs;

    invalidLIDs.push_back(0);
    for (uint32_t lid = 0x4; lid <= MAX_LID ; lid++)
        invalidLIDs.push_back(lid);

    return invalidLIDs;
}


}   // namespace
