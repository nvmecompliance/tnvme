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
#include "invalidFieldInCmd_r12.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/io.h"
#include "../Cmds/setFeatures.h"

namespace GrpAdminSetFeatCmd {


InvalidFieldInCmd_r12::InvalidFieldInCmd_r12(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_12)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.2, section 5");
    mTestDesc.SetShort(     "Issue reserved FID's; SC = Invalid field in cmd");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Create a baseline SetFeatures cmd, with no PRP payload. Issue cmd "
        "for all reserved DW10.FID = {0x00, 0x10 to 0x77, 0x81 to 0xBF}, "
        "expect failure.");
}


InvalidFieldInCmd_r12::~InvalidFieldInCmd_r12()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


InvalidFieldInCmd_r12::
InvalidFieldInCmd_r12(const InvalidFieldInCmd_r12 &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


InvalidFieldInCmd_r12 &
InvalidFieldInCmd_r12::operator=(const InvalidFieldInCmd_r12 &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
InvalidFieldInCmd_r12::RunnableCoreTest(bool preserve)
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
InvalidFieldInCmd_r12::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) none
     *  \endverbatim
     */
    string work;

    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Prepare the admin Q's");
    SharedACQPtr acq = SharedACQPtr(new ACQ(gDutFd));
    acq->Init(5);
    SharedASQPtr asq = SharedASQPtr(new ASQ(gDutFd));
    asq->Init(5);
    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    gCtrlrConfig->SetIOCQES(CtrlrConfig::MIN_IOCQES);
    gCtrlrConfig->SetIOSQES(CtrlrConfig::MIN_IOSQES);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Create Set features cmd");
    SharedSetFeaturesPtr setFeaturesCmd =
        SharedSetFeaturesPtr(new SetFeatures());

    LOG_NRM("Form a vector of invalid FID's");
    vector<uint16_t> invalidFIDs;
    uint8_t invalFID;
    
    invalidFIDs.push_back(0x00);
    invalidFIDs.push_back(0x0E);
    for (uint8_t invalFID = 0x10; invalFID <= 0x77; invalFID++)
        invalidFIDs.push_back(invalFID);

    if ((gInformative->GetIdentifyCmdCtrlr()->GetValue(IDCTRLRCAP_ONCS))
            & ONCS_SUP_RSRV)
        invalFID = 0x84;
    else
        invalFID = 0x81;  
 
    for (; invalFID <= 0xBF; invalFID++)
        invalidFIDs.push_back(invalFID);

    for (uint16_t i = 0; i < invalidFIDs.size(); i++) {
        if (invalidFIDs[i] == 0x81)
            continue;
        LOG_NRM("Issue set feat cmd using invalid FID = 0x%X", invalidFIDs[i]);
        setFeaturesCmd->SetFID(invalidFIDs[i]);
        work = str(boost::format("invalidFIDs.%xh") % invalidFIDs[i]);
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
            setFeaturesCmd, work, true, CESTAT_INVAL_FIELD);
    }
}

}   // namespace
