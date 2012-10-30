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
#include "fidIRQVec_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/acq.h"
#include "../Queues/asq.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/irq.h"
#include "../Utils/io.h"
#include "../Cmds/baseFeatures.h"
#include "../Cmds/getFeatures.h"
#include "../Cmds/setFeatures.h"

namespace GrpAdminSetGetFeatCombo {


FIDIRQVec_r10b::FIDIRQVec_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Verify changes are allowed on FID = IRQ vector");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Reset ctrlr to cause a clearing of DUT state. Issue GetFeatures, "
        "FID = 0x09. Programmatically and dynamically force new, legal values "
        "into all fields of CE.DW0 and assoc changes with SetFeatures. Redo "
        "GetFeatures to verify settings were accepted.");

}


FIDIRQVec_r10b::~FIDIRQVec_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


FIDIRQVec_r10b::
FIDIRQVec_r10b(const FIDIRQVec_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


FIDIRQVec_r10b &
FIDIRQVec_r10b::operator=(const FIDIRQVec_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
FIDIRQVec_r10b::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
FIDIRQVec_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * None.
     * \endverbatim
     */
    string work;
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

    LOG_NRM("Set and Get features for IRQ vec config (FID = 0x%x)",
        BaseFeatures::FID_IRQ_VEC_CONFIG);
    getFeaturesCmd->SetFID(BaseFeatures::FID_IRQ_VEC_CONFIG);
    setFeaturesCmd->SetFID(BaseFeatures::FID_IRQ_VEC_CONFIG);

    uint16_t max_ivec = IRQ::GetMaxIRQsSupportedAnyScheme();

    uint8_t ivecMismatch = 0;
    for (uint8_t cd = 0; cd < 2; cd++) {
         for (uint16_t ivec = 1; ivec < max_ivec; ivec++) {
            LOG_NRM("Set and Get features for IVECCONF # %d", ivec);

            LOG_NRM("Issue set features cmd with CD = %d", cd);
            LOG_NRM("Issue set features cmd with IVEC = %d", ivec);

            setFeaturesCmd->SetIntVecConfig(cd, ivec);
            work = str(boost::format("cdivec.%d") % ivec);
            IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
                asq, acq, setFeaturesCmd, work, true);

            getFeaturesCmd->SetIntVecConfigIV(ivec);
            acqMetrics = acq->GetQMetrics();
            LOG_NRM("Issue get features & check (cd, ivec)=(%d, %d)", cd, ivec);
            IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
                asq, acq, getFeaturesCmd, work, false);

            ce = acq->PeekCE(acqMetrics.head_ptr);
            LOG_NRM("IRQ Vector Conf using Get Features = %d", ce.t.dw0);
            if (ivec != (uint16_t)ce.t.dw0) {
                LOG_ERR("IRQ Vector Conf get feat does not match set feat"
                    "(expected, rcvd) = (%d, %d)", ivec, ce.t.dw0);
                ivecMismatch = 0xFF;
            }
            if (cd != ((ce.t.dw0 >> 16) & 0x1)) {
                LOG_ERR("Coalesing disable get feat does not match set feat"
                    "(expected, rcvd) = (%d, %d)", cd, (ce.t.dw0 >> 16) & 0x1);
                ivecMismatch = 0xFF;
            }
        }
    }

    if (ivecMismatch)
        throw FrmwkEx(HERE, "IRQ Vector Conf setting mismatched.");
}


}   // namespace
