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

#include "grpAdminSetGetFeatCombo.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/acq.h"
#include "../Queues/asq.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/irq.h"
#include "../Utils/io.h"
#include "../Cmds/getFeatures.h"
#include "../Cmds/setFeatures.h"
#include "fidArbitration_r10b.h"
#include "fidPwrMgmt_r10b.h"


namespace GrpAdminSetGetFeatCombo {


GrpAdminSetGetFeatCombo::GrpAdminSetGetFeatCombo(size_t grpNum) :
    Group(grpNum, "GrpAdminSetGetFeatCombo",
    "Admin cmd set/get features utilizing both cmds for efficiency.")
{
    // For complete details about the APPEND_TEST_AT_?LEVEL() macros:
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering" and
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Strategy
    switch (gCmdLine.rev) {
    case SPECREV_10b:
        APPEND_TEST_AT_XLEVEL(FIDArbitration_r10b, GrpAdminSetGetFeatCombo)
        APPEND_TEST_AT_XLEVEL(FIDPwrMgmt_r10b, GrpAdminSetGetFeatCombo)
        break;

    default:
    case SPECREVTYPE_FENCE:
        throw FrmwkEx(HERE, "Object created with an unknown SpecRev=%d",
            gCmdLine.rev);
    }
}


GrpAdminSetGetFeatCombo::~GrpAdminSetGetFeatCombo()
{
    // mTests deallocated in parent
}


bool
GrpAdminSetGetFeatCombo::SaveState()
{
    LOG_NRM("Saving the current arbitration state before the group starts.");

    // Reset saved value to account for regression
    arbStateSave = 0;
    psdStateSave = 0;

    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    SharedACQPtr acq = SharedACQPtr(new ACQ(gDutFd));
    acq->Init(5);
    SharedASQPtr asq = SharedASQPtr(new ASQ(gDutFd));
    asq->Init(5);

    // All queues will use identical IRQ vector
    IRQ::SetAnySchemeSpecifyNum(1);

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Create Get features cmd");
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());

    getFeaturesCmd->SetFID(BaseFeatures::FID_ARBITRATION);
    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        getFeaturesCmd, "SaveFeatArb", true);

    union CE ce = acq->PeekCE(acqMetrics.head_ptr);
    arbStateSave = ce.t.dw0;

    LOG_NRM("Default arbitration using Get Features = 0x%04X", arbStateSave);

    getFeaturesCmd->SetFID(BaseFeatures::FID_PWR_MGMT);
    acqMetrics = acq->GetQMetrics();
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        getFeaturesCmd, "SaveFeatPwrMgmt", true);
    ce = acq->PeekCE(acqMetrics.head_ptr);
    psdStateSave = ce.t.dw0;

    LOG_NRM("Default power state using Get Features = 0x%04X", psdStateSave);

    return true;
}


bool
GrpAdminSetGetFeatCombo::RestoreState()
{
    // For the majority of test groups this feature most likely won't be needed
     if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    SharedACQPtr acq = SharedACQPtr(new ACQ(gDutFd));
    acq->Init(5);

    SharedASQPtr asq = SharedASQPtr(new ASQ(gDutFd));
    asq->Init(5);

    // All queues will use identical IRQ vector
    IRQ::SetAnySchemeSpecifyNum(1);

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Create Set and get feature cmds");
    SharedSetFeaturesPtr setFeaturesCmd =
        SharedSetFeaturesPtr(new SetFeatures());
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());

    LOG_NRM("Restoring state with arbitration = 0x%04X", arbStateSave);
    setFeaturesCmd->SetFID(BaseFeatures::FID_ARBITRATION);
    getFeaturesCmd->SetFID(BaseFeatures::FID_ARBITRATION);

    setFeaturesCmd->SetDword(arbStateSave, 11);
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        setFeaturesCmd, "RestoreArb", true);

    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        getFeaturesCmd, "RestoreArb", true);
    union CE ce = acq->PeekCE(acqMetrics.head_ptr);

    if (arbStateSave != ce.t.dw0) {
        LOG_ERR("Arbitration restore to original state failed. "
            "(Actual: Expected) = (0x%04X:0x%04X)", ce.t.dw0, arbStateSave);
        return false;
    }

    LOG_NRM("Restoring state with PSD = 0x%04X", psdStateSave);
    setFeaturesCmd->SetFID(BaseFeatures::FID_PWR_MGMT);
    getFeaturesCmd->SetFID(BaseFeatures::FID_PWR_MGMT);

    setFeaturesCmd->SetDword(psdStateSave, 11);
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        setFeaturesCmd, "RestorePSD", true);

    acqMetrics = acq->GetQMetrics();
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        getFeaturesCmd, "RestorePSD", true);
    ce = acq->PeekCE(acqMetrics.head_ptr);

    if (psdStateSave != ce.t.dw0) {
        LOG_ERR("PSD restore to original state failed. "
            "(Actual: Expected) = (0x%04X:0x%04X)", ce.t.dw0, psdStateSave);
        return false;
    }

    LOG_NRM("System restore successful for arbitration and power management");
    return true;
}


}   // namespace
