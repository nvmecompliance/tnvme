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

#include <string.h>
#include "grpAdminSetGetFeatCombo.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/irq.h"
#include "../Utils/io.h"
#include "../Cmds/getFeatures.h"
#include "../Cmds/setFeatures.h"
#include "fidArbitration_r10b.h"
#include "fidPwrMgmt_r10b.h"
#include "fidTempThres_r10b.h"
#include "fidErrRecovery_r10b.h"
#include "fidVolatileCash_r10b.h"
#include "fidIRQCoalescing_r10b.h"
#include "fidIRQVec_r10b.h"
#include "fidWriteAtomicity_r10b.h"
#include "fidAsyncEventCfg_r10b.h"

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
        APPEND_TEST_AT_XLEVEL(FIDTempThres_r10b, GrpAdminSetGetFeatCombo)
        APPEND_TEST_AT_XLEVEL(FIDErrRecovery_r10b, GrpAdminSetGetFeatCombo)
        APPEND_TEST_AT_XLEVEL(FIDVolatileCash_r10b, GrpAdminSetGetFeatCombo)
        APPEND_TEST_AT_XLEVEL(FIDIRQCoalescing_r10b, GrpAdminSetGetFeatCombo)
        APPEND_TEST_AT_XLEVEL(FIDIRQVec_r10b, GrpAdminSetGetFeatCombo)
        APPEND_TEST_AT_XLEVEL(FIDWriteAtomicity_r10b, GrpAdminSetGetFeatCombo)
        APPEND_TEST_AT_XLEVEL(FIDAsyncEventCfg_r10b, GrpAdminSetGetFeatCombo)

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
    LOG_NRM("Saving system state using getfeatures before group starts");

    // Reset saved value to account for regression
    mArbitration = 0;
    mPowerState = 0;
    mTmpThreshold = 0;
    mTimeLimErrRec = 0;
    mVolWrCache = 0;
    mIrqCoalescing = 0;
    mWrAtomicity = 0;
    memset(mIvecConf, 0, sizeof(mIvecConf));
    mAsyncEvent = 0;

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

    SaveArbitration(asq, acq);
    SavePowerState(asq, acq);
    SaveTMPTH(asq, acq);
    SaveTLER(asq, acq);

    if ((gInformative->GetIdentifyCmdCtrlr()->GetValue(IDCTRLRCAP_VWC)
        & BITMASK_VWC) == 0x1) {
        SaveVolWrCache(asq, acq);
    }
    SaveIRQCoalescing(asq, acq);
    SaveIvecConf(asq, acq);
    SaveWrAtomicity(asq, acq);
    SaveAsyncEvent(asq, acq);
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

    if (RestoreArbitration(asq, acq) == false) {
        LOG_ERR("Arbitration restore failed");
        return false;
    }

    if (RestorePowerState(asq, acq) == false) {
        LOG_ERR("Power state restore failed");
        return false;
    }

    if (RestoreTMPTH(asq, acq) == false) {
        LOG_ERR("Temperature threshold restore failed");
        return false;
    }

    if (RestoreTLER(asq, acq) == false) {
        LOG_ERR("Time limited error recovery restore failed");
        return false;
    }

    if ((gInformative->GetIdentifyCmdCtrlr()->GetValue(IDCTRLRCAP_VWC)
        & BITMASK_VWC) == 0x1) {
        if (RestoreVolWrCache(asq, acq) == false) {
            LOG_ERR("Volatile write cache restore failed");
            return false;
        }
    }

    if (RestoreIRQCoalescing(asq, acq) == false) {
        LOG_ERR("IRQ Coalescing restore failed");
        return false;
    }

    if (RestoreSaveIvecConf(asq, acq) == false) {
        LOG_ERR("Interrupt vector configuration restore failed");
        return false;
    }

    if (RestoreWrAtomicity(asq, acq) == false) {
        LOG_ERR("Write Atomicity restore failed");
        return false;
    }

    if (RestoreAsyncEvent(asq, acq) == false) {
        LOG_ERR("Async events restore failed");
        return false;
    }

    LOG_NRM("System restore successful.");
    return true;
}


void
GrpAdminSetGetFeatCombo::SaveArbitration(SharedASQPtr asq, SharedACQPtr acq)
{
    LOG_NRM("Create Get features cmd");
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());
    getFeaturesCmd->SetFID(BaseFeatures::FID_ARBITRATION);
    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        getFeaturesCmd, "SaveFeatArb", true);

    union CE ce = acq->PeekCE(acqMetrics.head_ptr);
    mArbitration = ce.t.dw0;
    LOG_NRM("Default arbitration using Get Features = 0x%04X", mArbitration);
}


void
GrpAdminSetGetFeatCombo::SavePowerState(SharedASQPtr asq, SharedACQPtr acq)
{
    LOG_NRM("Create Get features cmd");
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());
    getFeaturesCmd->SetFID(BaseFeatures::FID_PWR_MGMT);
    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        getFeaturesCmd, "SaveFeatPwrMgmt", true);
    union CE ce = acq->PeekCE(acqMetrics.head_ptr);
    mPowerState = ce.t.dw0;
    LOG_NRM("Default power state using Get Features = 0x%04X", mPowerState);
}


void
GrpAdminSetGetFeatCombo::SaveTMPTH(SharedASQPtr asq, SharedACQPtr acq)
{
    LOG_NRM("Create Get features cmd");
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());
    getFeaturesCmd->SetFID(BaseFeatures::FID_TEMP_THRESHOLD);
    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        getFeaturesCmd, "SaveFeatTmpThr", true);
    union CE ce = acq->PeekCE(acqMetrics.head_ptr);
    mTmpThreshold = ce.t.dw0;
    LOG_NRM("Default tmp threshold using Get Features = 0x%04X", mTmpThreshold);
}


void
GrpAdminSetGetFeatCombo::SaveTLER(SharedASQPtr asq, SharedACQPtr acq)
{
    LOG_NRM("Create Get features cmd");
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());
    getFeaturesCmd->SetFID(BaseFeatures::FID_ERR_RECOVERY);
    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        getFeaturesCmd, "SaveFeatTler", true);
    union CE ce = acq->PeekCE(acqMetrics.head_ptr);
    mTimeLimErrRec = ce.t.dw0;
    LOG_NRM("Default time limited err recovery using Get Features = 0x%04X",
        mTimeLimErrRec);
}


void
GrpAdminSetGetFeatCombo::SaveVolWrCache(SharedASQPtr asq, SharedACQPtr acq)
{
    LOG_NRM("Create Get features cmd");
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());

    getFeaturesCmd->SetFID(BaseFeatures::FID_VOL_WR_CACHE);
    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        getFeaturesCmd, "SaveFeatVWC", true);
    union CE ce = acq->PeekCE(acqMetrics.head_ptr);
    mVolWrCache = ce.t.dw0;
    LOG_NRM("Default volatile write cache using Get Features = 0x%04X",
        mVolWrCache);
}


void
GrpAdminSetGetFeatCombo::SaveIRQCoalescing(SharedASQPtr asq, SharedACQPtr acq)
{
    LOG_NRM("Create Get features cmd");
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());

    getFeaturesCmd->SetFID(BaseFeatures::FID_IRQ_COALESCING);
    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        getFeaturesCmd, "SaveFeatIRQCoalescing", true);
    union CE ce = acq->PeekCE(acqMetrics.head_ptr);
    mIrqCoalescing = ce.t.dw0;
    LOG_NRM("Default irq coalescing using Get Features = 0x%04X",
        mIrqCoalescing);
}


void
GrpAdminSetGetFeatCombo::SaveIvecConf(SharedASQPtr asq, SharedACQPtr acq)
{
    LOG_NRM("Create Get features cmd");
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());

    getFeaturesCmd->SetFID(BaseFeatures::FID_IRQ_VEC_CONFIG);

    uint16_t max_ivec = IRQ::GetMaxIRQsSupportedAnyScheme();

    for (uint16_t ivec = 0; ivec < max_ivec; ivec++) {
        getFeaturesCmd->SetIntVecConfigIV(ivec);
        struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
        IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
            getFeaturesCmd, "SaveFeatIvecCOnf", true);
        union CE ce = acq->PeekCE(acqMetrics.head_ptr);
        mIvecConf[ivec] = ce.t.dw0;
        LOG_NRM("Default conf = 0x%04X using Get Features for ivec = 0x%02X",
            mIvecConf[ivec], ivec);
    }
}


void
GrpAdminSetGetFeatCombo::SaveWrAtomicity(SharedASQPtr asq, SharedACQPtr acq)
{
    LOG_NRM("Create Get features cmd");
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());

    getFeaturesCmd->SetFID(BaseFeatures::FID_WRITE_ATOMICITY);
    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        getFeaturesCmd, "SaveFeatWrAtomicity", true);
    union CE ce = acq->PeekCE(acqMetrics.head_ptr);
    mWrAtomicity = ce.t.dw0;
    LOG_NRM("Default WrAtomicity using Get Features = 0x%04X", mWrAtomicity);
}


void
GrpAdminSetGetFeatCombo::SaveAsyncEvent(SharedASQPtr asq, SharedACQPtr acq)
{
    LOG_NRM("Create Get features cmd");
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());

    getFeaturesCmd->SetFID(BaseFeatures::FID_ASYNC_EVENT_CONFIG);
    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        getFeaturesCmd, "SaveFeatAsyncEvent", true);
    union CE ce = acq->PeekCE(acqMetrics.head_ptr);
    mAsyncEvent = ce.t.dw0;
    LOG_NRM("Default Async Events using Get Features = 0x%04X", mAsyncEvent);
}


bool
GrpAdminSetGetFeatCombo::RestoreArbitration(SharedASQPtr asq, SharedACQPtr acq)
{
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());
    SharedSetFeaturesPtr setFeaturesCmd =
        SharedSetFeaturesPtr(new SetFeatures());

    LOG_NRM("Restoring state with arbitration = 0x%04X", mArbitration);
    setFeaturesCmd->SetFID(BaseFeatures::FID_ARBITRATION);
    getFeaturesCmd->SetFID(BaseFeatures::FID_ARBITRATION);

    setFeaturesCmd->SetDword(mArbitration, 11);
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        setFeaturesCmd, "RestoreArb", true);

    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        getFeaturesCmd, "RestoreArb", true);
    union CE ce = acq->PeekCE(acqMetrics.head_ptr);

    if (mArbitration != ce.t.dw0) {
        LOG_ERR("Arbitration restore to original state failed. "
            "(Actual: Expected) = (0x%04X:0x%04X)", ce.t.dw0, mArbitration);
        return false;
    }
    return true;
}


bool
GrpAdminSetGetFeatCombo::RestorePowerState(SharedASQPtr asq, SharedACQPtr acq)
{
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());
    SharedSetFeaturesPtr setFeaturesCmd =
        SharedSetFeaturesPtr(new SetFeatures());

    LOG_NRM("Restoring state with PSD = 0x%04X", mPowerState);
    setFeaturesCmd->SetFID(BaseFeatures::FID_PWR_MGMT);
    getFeaturesCmd->SetFID(BaseFeatures::FID_PWR_MGMT);

    setFeaturesCmd->SetDword(mPowerState, 11);
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        setFeaturesCmd, "RestorePSD", true);

    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        getFeaturesCmd, "RestorePSD", true);
    union CE ce = acq->PeekCE(acqMetrics.head_ptr);

    if (mPowerState != ce.t.dw0) {
        LOG_ERR("PSD restore to original state failed. "
            "(Actual: Expected) = (0x%04X:0x%04X)", ce.t.dw0, mPowerState);
        return false;
    }
    return true;
}


bool
GrpAdminSetGetFeatCombo::RestoreTMPTH(SharedASQPtr asq, SharedACQPtr acq)
{
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());
    SharedSetFeaturesPtr setFeaturesCmd =
        SharedSetFeaturesPtr(new SetFeatures());

    LOG_NRM("Restoring state with TMPTH = 0x%04X", mTmpThreshold);
    setFeaturesCmd->SetFID(BaseFeatures::FID_TEMP_THRESHOLD);
    getFeaturesCmd->SetFID(BaseFeatures::FID_TEMP_THRESHOLD);

    setFeaturesCmd->SetDword(mTmpThreshold, 11);
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        setFeaturesCmd, "RestoreTMPTH", true);

    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        getFeaturesCmd, "RestoreTMPTH", true);
    union CE ce = acq->PeekCE(acqMetrics.head_ptr);

    if (mTmpThreshold != ce.t.dw0) {
        LOG_ERR("TMPTH restore to original state failed. "
            "(Actual: Expected) = (0x%04X:0x%04X)", ce.t.dw0, mTmpThreshold);
        return false;
    }
    return true;
}


bool
GrpAdminSetGetFeatCombo::RestoreTLER(SharedASQPtr asq, SharedACQPtr acq)
{
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());
    SharedSetFeaturesPtr setFeaturesCmd =
        SharedSetFeaturesPtr(new SetFeatures());

    LOG_NRM("Restoring state with TLER = 0x%04X", mTimeLimErrRec);
    setFeaturesCmd->SetFID(BaseFeatures::FID_ERR_RECOVERY);
    getFeaturesCmd->SetFID(BaseFeatures::FID_ERR_RECOVERY);

    setFeaturesCmd->SetDword(mTimeLimErrRec, 11);
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        setFeaturesCmd, "RestoreTLER", true);

    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        getFeaturesCmd, "RestoreTLER", true);
    union CE ce = acq->PeekCE(acqMetrics.head_ptr);

    if (mTimeLimErrRec != ce.t.dw0) {
        LOG_ERR("TLER restore to original state failed. "
            "(Actual: Expected) = (0x%04X:0x%04X)", ce.t.dw0, mTimeLimErrRec);
        return false;
    }
    return true;
}


bool
GrpAdminSetGetFeatCombo::RestoreVolWrCache(SharedASQPtr asq, SharedACQPtr acq)
{
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());
    SharedSetFeaturesPtr setFeaturesCmd =
        SharedSetFeaturesPtr(new SetFeatures());

    LOG_NRM("Restoring state with VWC = 0x%04X", mVolWrCache);
    setFeaturesCmd->SetFID(BaseFeatures::FID_VOL_WR_CACHE);
    getFeaturesCmd->SetFID(BaseFeatures::FID_VOL_WR_CACHE);

    setFeaturesCmd->SetDword(mVolWrCache, 11);
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        setFeaturesCmd, "RestoreVWC", true);

    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        getFeaturesCmd, "RestoreVWC", true);
    union CE ce = acq->PeekCE(acqMetrics.head_ptr);

    if (mVolWrCache != ce.t.dw0) {
        LOG_ERR("VWC restore to original state failed. "
            "(Actual: Expected) = (0x%04X:0x%04X)", ce.t.dw0, mVolWrCache);
        return false;
    }
    return true;
}


bool
GrpAdminSetGetFeatCombo::RestoreIRQCoalescing(SharedASQPtr asq,
    SharedACQPtr acq)
{
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());
    SharedSetFeaturesPtr setFeaturesCmd =
        SharedSetFeaturesPtr(new SetFeatures());

    LOG_NRM("Restoring state with IRQCoalescing = 0x%04X", mIrqCoalescing);
    setFeaturesCmd->SetFID(BaseFeatures::FID_IRQ_COALESCING);
    getFeaturesCmd->SetFID(BaseFeatures::FID_IRQ_COALESCING);

    setFeaturesCmd->SetDword(mIrqCoalescing, 11);
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        setFeaturesCmd, "RestoreIRQCoalescing", true);

    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        getFeaturesCmd, "RestoreIRQCoalescing", true);
    union CE ce = acq->PeekCE(acqMetrics.head_ptr);

    if (mIrqCoalescing != ce.t.dw0) {
        LOG_ERR("IRQCoalescing restore to original state failed. "
            "(Actual: Expected) = (0x%04X:0x%04X)", ce.t.dw0, mIrqCoalescing);
        return false;
    }
    return true;
}


bool
GrpAdminSetGetFeatCombo::RestoreSaveIvecConf(SharedASQPtr asq, SharedACQPtr acq)
{
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());
    SharedSetFeaturesPtr setFeaturesCmd =
        SharedSetFeaturesPtr(new SetFeatures());

    uint16_t max_ivec = IRQ::GetMaxIRQsSupportedAnyScheme();

    for (uint16_t ivec = 0; ivec < max_ivec; ivec++) {
        LOG_NRM("Restoring state for ivec = 0x%02X with IvecConf = 0x%04X",
            ivec, mIvecConf[ivec]);
        setFeaturesCmd->SetFID(BaseFeatures::FID_IRQ_VEC_CONFIG);
        getFeaturesCmd->SetFID(BaseFeatures::FID_IRQ_VEC_CONFIG);

        setFeaturesCmd->SetDword(mIvecConf[ivec], 11);
        IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
            setFeaturesCmd, "RestoreIvecConf", true);

        getFeaturesCmd->SetIntVecConfigIV(ivec);
        struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
        IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
            getFeaturesCmd, "RestoreIvecConf", true);
        union CE ce = acq->PeekCE(acqMetrics.head_ptr);

        if (mIvecConf[ivec] != ce.t.dw0) {
            LOG_ERR("mIvecConf restore to original state failed. "
                "(Actual: Expected) = (0x%04X:0x%04X)", ce.t.dw0,
                mIvecConf[ivec]);
            return false;
        }
    }

    return true;
}


bool
GrpAdminSetGetFeatCombo::RestoreWrAtomicity(SharedASQPtr asq, SharedACQPtr acq)
{
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());
    SharedSetFeaturesPtr setFeaturesCmd =
        SharedSetFeaturesPtr(new SetFeatures());

    LOG_NRM("Restoring state with WrAtomicity = 0x%04X", mWrAtomicity);
    setFeaturesCmd->SetFID(BaseFeatures::FID_WRITE_ATOMICITY);
    getFeaturesCmd->SetFID(BaseFeatures::FID_WRITE_ATOMICITY);

    setFeaturesCmd->SetDword(mWrAtomicity, 11);
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        setFeaturesCmd, "RestoreWrAtomicity", true);

    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        getFeaturesCmd, "RestoreWrAtomicity", true);
    union CE ce = acq->PeekCE(acqMetrics.head_ptr);

    if (mWrAtomicity != ce.t.dw0) {
        LOG_ERR("Write Atomicity restore to original state failed. "
            "(Actual: Expected) = (0x%04X:0x%04X)", ce.t.dw0, mWrAtomicity);
        return false;
    }
    return true;
}


bool
GrpAdminSetGetFeatCombo::RestoreAsyncEvent(SharedASQPtr asq, SharedACQPtr acq)
{
    SharedGetFeaturesPtr getFeaturesCmd =
        SharedGetFeaturesPtr(new GetFeatures());
    SharedSetFeaturesPtr setFeaturesCmd =
        SharedSetFeaturesPtr(new SetFeatures());

    LOG_NRM("Restoring state with mAsyncEvent = 0x%04X", mAsyncEvent);
    setFeaturesCmd->SetFID(BaseFeatures::FID_ASYNC_EVENT_CONFIG);
    getFeaturesCmd->SetFID(BaseFeatures::FID_ASYNC_EVENT_CONFIG);

    setFeaturesCmd->SetDword(mAsyncEvent, 11);
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        setFeaturesCmd, "RestoreWrAtomicity", true);

    struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
    IO::SendAndReapCmd(mGrpName, mGrpName, CALC_TIMEOUT_ms(1), asq, acq,
        getFeaturesCmd, "RestoreAsyncEvent", true);
    union CE ce = acq->PeekCE(acqMetrics.head_ptr);

    if (mAsyncEvent != ce.t.dw0) {
        LOG_ERR("AsyncEvent restore to original state failed. "
            "(Actual: Expected) = (0x%04X:0x%04X)", ce.t.dw0, mAsyncEvent);
        return false;
    }
    return true;
}


}   // namespace
