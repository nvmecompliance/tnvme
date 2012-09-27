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
#include "invalidNamspc_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/io.h"
#include "../Cmds/getLogPage.h"

#define SMART_HEALTH_INFO_LID       0x02
#define SMART_NUMD                  (514 / 4)

namespace GrpAdminGetLogPgCmd {


InvalidNamspc_r10b::InvalidNamspc_r10b
    (string grpName, string testName) : Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Issue GetLogPage and cause SC=Invalid Namspc or Format");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Issue GetLogPage cmd, LID=2, NUMD=(512/4). Determine Identify.LPA.0, "
        "and if SMART is supported, then determine X=Identity.NN and loop "
        "GetLogPage to all namspcs not supported by DUT and expect failure, "
        "0xFFFFFFFF is legal and return global page. If SMART "
        "is not supported then all NSID values are legal, verify success.");
}


InvalidNamspc_r10b::~InvalidNamspc_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


InvalidNamspc_r10b::
InvalidNamspc_r10b(const InvalidNamspc_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


InvalidNamspc_r10b &
InvalidNamspc_r10b::operator=(const InvalidNamspc_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
InvalidNamspc_r10b::RunnableCoreTest(bool preserve)
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
InvalidNamspc_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     *  \endverbatim
     */
    string work;
    uint64_t inc, i;

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    LOG_NRM("Create get log page cmd and assoc some buffer memory");
    SharedGetLogPagePtr getLogPgCmd = SharedGetLogPagePtr(new GetLogPage());
    SharedMemBufferPtr getLogPageMem = SharedMemBufferPtr(new MemBuffer());
    send_64b_bitmask prpReq =
            (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);

    getLogPageMem->InitOffset1stPage(GetLogPage::SMART_DATA_SIZE, 0, true);
    getLogPgCmd->SetPrpBuffer(prpReq, getLogPageMem);
    getLogPgCmd->SetLID(SMART_HEALTH_INFO_LID);
    getLogPgCmd->SetNUMD(SMART_NUMD - 1); // 0 - based

    ConstSharedIdentifyPtr idCtrlrStruct = gInformative->GetIdentifyCmdCtrlr();
    uint32_t nn = (uint32_t)idCtrlrStruct->GetValue(IDCTRLRCAP_NN);
    if (nn == 0 )
        throw FrmwkEx(HERE, "Required to support >= 1 namespace");

    uint8_t logPageAttr = (uint8_t)idCtrlrStruct->GetValue(IDCTRLRCAP_LPA);

    // for all illegal nsid's verify get log page cmd.
    for (i = (nn + 1), inc = 1; i < 0xffffffff; i += (2 * inc), inc += 1327) {
        LOG_NRM("Issue Get log page cmd with illegal namspc ID = %llu",
            (unsigned long long)i);
        getLogPgCmd->SetNSID(i);

        work = str(boost::format("namspc%d") % i);
        // Determine the status based on the per namespace mask bit.
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
            getLogPgCmd, work, true, logPageAttr ? CESTAT_INVAL_NAMSPC :
            CESTAT_SUCCESS);
    }

    // If per namespc smart log is supported, check all legal nn.
    if (logPageAttr) {
        for (i = 1; i <= nn; i++) {
            LOG_NRM("Issue Get log page cmd with legal namspc ID = %llu",
                (unsigned long long)i);
            getLogPgCmd->SetNSID(i);

            work = str(boost::format("namspc%d") % i);
            // Determine the status based on the per namespace mask bit.
            IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
                asq, acq, getLogPgCmd, work, true);
        }
    } else {
        i = 0;
        getLogPgCmd->SetNSID(i);
        work = str(boost::format("namspc%d") % i);
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
            getLogPgCmd, work, true);
    }

    // If name space is 0xffffffff, then we should get the global log info.
    i = 0xffffffff;
    getLogPgCmd->SetNSID(i);
    work = str(boost::format("namspc%d") % i);
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
        getLogPgCmd, work, true);
}


}   // namespace
