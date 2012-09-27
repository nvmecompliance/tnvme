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

#include "prp1PRP2_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/kernelAPI.h"
#include "../Singletons/informative.h"
#include "../Queues/acq.h"
#include "../Queues/asq.h"
#include "../Utils/io.h"

#define PRP_BUFFER_OFFSET       0x800

namespace GrpAdminIdentifyCmd {


PRP1PRP2_r10b::PRP1PRP2_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Request data using PRP1 and PRP2");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Issue Identify cmd requesting ctrlr namspc struct utilizing PRP1 & "
        "PRP2 for the user space buffer, and expect success.");
}


PRP1PRP2_r10b::~PRP1PRP2_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


PRP1PRP2_r10b::
PRP1PRP2_r10b(const PRP1PRP2_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


PRP1PRP2_r10b &
PRP1PRP2_r10b::operator=(const PRP1PRP2_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
PRP1PRP2_r10b::RunnableCoreTest(bool preserve)
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
PRP1PRP2_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */

    LOG_NRM("Lookup objs which were created in a prior test within group");
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    LOG_NRM("Determine if DUT has atleast one namespace support");
    ConstSharedIdentifyPtr idCmdCtrlr = gInformative->GetIdentifyCmdCtrlr();
    if ((idCmdCtrlr->GetValue(IDCTRLRCAP_NN)) == 0)
        throw FrmwkEx(HERE, "Required to support >= 1 namespace");

    LOG_NRM("Form identify namespace cmd and associate some buffer");
    SharedIdentifyPtr idCmdNamSpc = SharedIdentifyPtr(new Identify());
    idCmdNamSpc->SetCNS(false);
    idCmdNamSpc->SetNSID(1);

    SharedMemBufferPtr idMemNamSpc = SharedMemBufferPtr(new MemBuffer());
    idMemNamSpc->InitOffset1stPage(Identify::IDEAL_DATA_SIZE,
        PRP_BUFFER_OFFSET, true);

    LOG_NRM("Allow PRP1 and PRP2");
    send_64b_bitmask idPrpNamSpc =
        (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    idCmdNamSpc->SetPrpBuffer(idPrpNamSpc, idMemNamSpc);

    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
        idCmdNamSpc, "prp1Prp2", true);
}

}   // namespace
