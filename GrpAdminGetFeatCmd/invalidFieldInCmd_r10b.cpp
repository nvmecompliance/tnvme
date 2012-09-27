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
#include "invalidFieldInCmd_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/io.h"
#include "../Cmds/getFeatures.h"

namespace GrpAdminGetFeatCmd {


InvalidFieldInCmd_r10b::InvalidFieldInCmd_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Issue reserved FID's; SC = Invalid field in cmd");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Create a baseline GetFeatures cmd, with no PRP payload. Issue cmd "
        "for all reserved DW10.FID = {0x00, 0x0C to 0x7F, 0x81 to 0xBF}, "
        "expect failure.");
}


InvalidFieldInCmd_r10b::~InvalidFieldInCmd_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


InvalidFieldInCmd_r10b::
InvalidFieldInCmd_r10b(const InvalidFieldInCmd_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


InvalidFieldInCmd_r10b &
InvalidFieldInCmd_r10b::operator=(const InvalidFieldInCmd_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
InvalidFieldInCmd_r10b::RunnableCoreTest(bool preserve)
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
InvalidFieldInCmd_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */
    string work;

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    LOG_NRM("Create Get features cmd");
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());

    LOG_NRM("Form a vector of invalid FID's");
    vector<uint16_t> invalidFIDs;
    invalidFIDs.push_back(0x00);
    for (uint8_t invalFID = 0x0C; invalFID <= 0x7F; invalFID++)
        invalidFIDs.push_back(invalFID);

    for (uint8_t invalFID = 0x81; invalFID <= 0xBF; invalFID++)
        invalidFIDs.push_back(invalFID);

    for (uint16_t i = 0; i < invalidFIDs.size(); i++) {
        LOG_NRM("Issue get feat cmd using invalid FID = 0x%X", invalidFIDs[i]);
        getFeaturesCmd->SetFID(invalidFIDs[i]);
        work = str(boost::format("invalidFIDs.%xh") % invalidFIDs[i]);
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
            getFeaturesCmd, work, true, CESTAT_INVAL_FIELD);
    }
}

}   // namespace
