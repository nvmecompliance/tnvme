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
#include "fidArbitration_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/acq.h" 
#include "../Queues/asq.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/irq.h"
#include "../Utils/io.h"
#include "../Cmds/getFeatures.h"
#include "../Cmds/setFeatures.h"

#define NO_LIMIT            0x7

namespace GrpAdminSetGetFeatCombo {


FIDArbitration_r10b::FIDArbitration_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Verify changes are allowed on FID = Arbitration");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Reset ctrlr to cause a clearing of DUT state. Issue GetFeatures, "
        "FID = 0x01. Programmatically and dynamically force new, legal values "
        "into all fields of CE.DW0 and assoc changes with SetFeatures. "
        "Redo GetFeatures to verify settings were accepted.");

}


FIDArbitration_r10b::~FIDArbitration_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


FIDArbitration_r10b::
FIDArbitration_r10b(const FIDArbitration_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


FIDArbitration_r10b &
FIDArbitration_r10b::operator=(const FIDArbitration_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
FIDArbitration_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
FIDArbitration_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * None.
     * \endverbatim
     */
    string work;
    uint32_t arbValDW11 = 0;
    union CE ce;
    struct nvme_gen_cq acqMetrics;

    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Create admin queues ACQ and ASQ for test lifetime");
    SharedACQPtr acq = SharedACQPtr(new ACQ(gDutFd));
    acq->Init(5);

    SharedASQPtr asq = SharedASQPtr(new ASQ(gDutFd));
    asq->Init(5);

    // All queues will use identical IRQ vector
    IRQ::SetAnySchemeSpecifyNum(1);

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Create Get features and set features cmds");
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());
    SharedSetFeaturesPtr setFeaturesCmd =
        SharedSetFeaturesPtr(new SetFeatures());

    LOG_NRM("Set and Get features arbitration (FID = 0x%x)",
        BaseFeatures::FID_ARBITRATION);
    getFeaturesCmd->SetFID(BaseFeatures::FID_ARBITRATION);
    setFeaturesCmd->SetFID(BaseFeatures::FID_ARBITRATION);

    uint8_t arbBurst[] =  {
        1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, NO_LIMIT
    };
    uint64_t arbSize = sizeof(arbBurst) / sizeof(arbBurst[0]);

    for (uint64_t arbVal = 0, inc = 1; arbVal < 0xFFFFFFFF;
        arbVal += (2 * inc), inc += 1327) {
        LOG_NRM("Prepare arbitration value for loop # %ld", arbVal);
        arbValDW11 = arbVal & 0xFFFFFF00;
        arbValDW11 |= arbBurst[arbVal % arbSize];

        LOG_NRM("Issue set features with arb setting (DW11)= 0x%X", arbValDW11);
        setFeaturesCmd->SetArbitrationHPW(arbValDW11 >> 24);
        setFeaturesCmd->SetArbitrationMPW(arbValDW11 >> 16);
        setFeaturesCmd->SetArbitrationLPW(arbValDW11 >> 8);
        setFeaturesCmd->SetArbitrationAB(arbValDW11);
        work = str(boost::format("arbValDW11.%xh") % arbValDW11);
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
            setFeaturesCmd, work, false);

        acqMetrics = acq->GetQMetrics();

        LOG_NRM("Issue get features cmd and check for arb = 0x%X", arbValDW11);
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
            getFeaturesCmd, work, false);

        ce = acq->PeekCE(acqMetrics.head_ptr);
        LOG_NRM("Arbitration status using Get Features = 0x%04X", ce.t.dw0);
        if (arbValDW11 != ce.t.dw0) {
            throw FrmwkEx(HERE, "Arbitration get feat does not match set feat"
                "(expected, rcvd) = (0x%04X, 0x%04X)", arbValDW11, ce.t.dw0);
        }
    }
}


}   // namespace
