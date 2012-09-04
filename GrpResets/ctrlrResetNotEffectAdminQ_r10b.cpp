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

#include "ctrlrResetNotEffectAdminQ_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/io.h"

namespace GrpResets {

#define IOCQ_ID                     1
#define IOSQ_ID                     2


CtrlrResetNotEffectAdminQ_r10b::CtrlrResetNotEffectAdminQ_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Ctrlr resets do not affect ACQ or ASQ.");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Create ACQ & ASQ, cause (CC.EN=0), then re-enable ctrlr, verify the "
        "admin Q's are still operation by sending an Identify and verifying it "
        "completes successfully.");
}


CtrlrResetNotEffectAdminQ_r10b::~CtrlrResetNotEffectAdminQ_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


CtrlrResetNotEffectAdminQ_r10b::
CtrlrResetNotEffectAdminQ_r10b(const CtrlrResetNotEffectAdminQ_r10b &other) :
    Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


CtrlrResetNotEffectAdminQ_r10b &
CtrlrResetNotEffectAdminQ_r10b::operator=(
    const CtrlrResetNotEffectAdminQ_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
CtrlrResetNotEffectAdminQ_r10b::RunnableCoreTest(bool preserve)
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
CtrlrResetNotEffectAdminQ_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) none
     *  \endverbatim
     */
    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    // Create Admin Q Objects with test lifetime
    SharedACQPtr acq = SharedACQPtr(new ACQ(gDutFd));
    acq->Init(15);
    SharedASQPtr asq = SharedASQPtr(new ASQ(gDutFd));
    asq->Init(15);

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);

    SendIdentifyCtrlrStruct(asq, acq);

    LOG_NRM("CC.EN=0, does not reset AQA/ASQ/ACQ registers");
    if (gCtrlrConfig->SetState(ST_DISABLE) == false)
        throw FrmwkEx(HERE);
    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);

    SendIdentifyCtrlrStruct(asq, acq);
}


void
CtrlrResetNotEffectAdminQ_r10b::SendIdentifyCtrlrStruct(SharedASQPtr asq,
    SharedACQPtr acq)
{
    LOG_NRM("Create 1st identify cmd and assoc some buffer memory");
    SharedIdentifyPtr idCmdCtrlr = SharedIdentifyPtr(new Identify());
    LOG_NRM("Force identify to request ctrlr capabilities struct");
    idCmdCtrlr->SetCNS(true);
    SharedMemBufferPtr idMemCap = SharedMemBufferPtr(new MemBuffer());
    idMemCap->InitAlignment(Identify::IDEAL_DATA_SIZE, PRP_BUFFER_ALIGNMENT,
        false, 0);
    send_64b_bitmask prpReq =
        (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    idCmdCtrlr->SetPrpBuffer(prpReq, idMemCap);

    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
        idCmdCtrlr, "IdCtrlStruct", true);
}


}   // namespace
