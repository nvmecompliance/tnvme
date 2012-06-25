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

#include "ctrlrResetDefaults_r10b.h"
#include "globals.h"
#include "../Utils/kernelAPI.h"

namespace GrpCtrlRegisters {


CtrlrResetDefaults_r10b::CtrlrResetDefaults_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 3");
    mTestDesc.SetShort(     "Verify approp registers are reset to default values");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "All registers within the ctrl'r register space must be reset to "
        "default values after a CC.EN=0 action including the doorbell regs, "
        "except the following: AQA, ASQ, and ACQ. An attempt to change the "
        "value of ctrl'r space registers which can be changed, then issue a "
        "reset; must yield this verifiable state.");
}


CtrlrResetDefaults_r10b::~CtrlrResetDefaults_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


CtrlrResetDefaults_r10b::
CtrlrResetDefaults_r10b(const CtrlrResetDefaults_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


CtrlrResetDefaults_r10b &
CtrlrResetDefaults_r10b::operator=(const CtrlrResetDefaults_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
CtrlrResetDefaults_r10b::RunnableCoreTest(bool preserve)
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
CtrlrResetDefaults_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) none
     *  \endverbatim
     */
    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    VerifyCtrlrResetDefaults();
}

void
CtrlrResetDefaults_r10b::VerifyCtrlrResetDefaults()
{
    std::map<int, uint64_t> ctrlRegsMap;

    // Issue a cntl'r reset for a clean controller state to begin with
    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    // Create ACQ and ASQ objects which have test life time
    SharedACQPtr acq = CAST_TO_ACQ(SharedACQPtr(new ACQ(gDutFd)))
    acq->Init(5);

    SharedASQPtr asq = CAST_TO_ASQ(SharedASQPtr(new ASQ(gDutFd)))
    asq->Init(5);

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);

    // Dump Controller space registers to known file before ctrlr reset
    KernelAPI::DumpCtrlrSpaceRegs(FileSystem::
        PrepDumpFile(mGrpName, mTestName, "ctrlRegs", "beforeReset"), false);

    ModifyRWBitsOfCtrlrRegisters(ctrlRegsMap);

    // Issue a cntl'r reset. CC.EN transitions from 1 to 0
    if (gCtrlrConfig->SetState(ST_DISABLE) == false)
        throw FrmwkEx(HERE);

    // Dump Controller space registers to known file after ctrlr reset
    KernelAPI::DumpCtrlrSpaceRegs(FileSystem::
        PrepDumpFile(mGrpName, mTestName, "ctrlRegs", "afterReset"), false);

    ValidateCtrlrRWDefaultsAfterReset(ctrlRegsMap);
}

void
CtrlrResetDefaults_r10b::ModifyRWBitsOfCtrlrRegisters(std::map <int, uint64_t> &
    ctrlRegsMap)
{
    uint64_t value = 0;
    const CtlSpcType *ctlMetrics = gRegisters->GetCtlMetrics();

    /// Traverse the ctrl'r registers
    for (int j = 0; j < CTLSPC_FENCE; j++) {
        if (ctlMetrics[j].specRev != mSpecRev)
            continue;

        // Reserved areas at NOT suppose to be written, so skip these
        // registers
        if ((j == CTLSPC_RES1) || (j == CTLSPC_RES2) || (j == CTLSPC_RES3))
            continue;

        // Interrupt mask clear register should not written as writing
        // to this register will clear interrupt mask set register.
        if (j == CTLSPC_INTMC)
            continue;

        // AQA, ASQ and ACQ should not be modified but remembered so that
        // we can compare after a cntl'r reset is issued
        if ((j == CTLSPC_AQA) || (j == CTLSPC_ASQ) || (j == CTLSPC_ACQ)) {
            if (gRegisters->Read((CtlSpc)j, value) == false)
                throw FrmwkEx(HERE);
            ctrlRegsMap[j] = value;
            continue;
        }

        // Modify RW bits in controller registers by writing non-default
        // values.
        if (gRegisters->Write((CtlSpc)j, ~ctlMetrics[j].dfltValue) == false)
            throw FrmwkEx(HERE);
    }
}

void
CtrlrResetDefaults_r10b::ValidateCtrlrRWDefaultsAfterReset(std::map <int,
    uint64_t>& ctrlRegsMap)
{
    uint64_t value = 0;
    uint64_t expectedVal = 0;
    const CtlSpcType *ctlMetrics = gRegisters->GetCtlMetrics();

    /// Traverse the ctrl'r registers
    for (int j = 0; j < CTLSPC_FENCE; j++) {
        if (ctlMetrics[j].specRev != mSpecRev)
            continue;

        // Reserved areas at NOT suppose to be written so not validated
        if ((j == CTLSPC_RES1) || (j == CTLSPC_RES2) || (j == CTLSPC_RES3))
            continue;

        // AQA, ASQ and ACQ register vals are checked against remembered values
        if ((j == CTLSPC_AQA) || (j == CTLSPC_ASQ) || (j == CTLSPC_ACQ)) {
            if (gRegisters->Read((CtlSpc)j, value) == false)
                throw FrmwkEx(HERE);
            if (value != ctrlRegsMap[j]) {
                LOG_ERR("%s is incorrectly modified expected value is 0x%lX "
                "but actual value 0x%lX", ctlMetrics[j].desc, ctrlRegsMap[j],
                value);
                throw FrmwkEx(HERE);
            }
            continue;
        }

        // Validate RW bits are reset to default values
        if (gRegisters->Read((CtlSpc)j, value) == false)
            throw FrmwkEx(HERE);
        value &= ~ctlMetrics[j].maskRO;
        if (value != expectedVal) {
            throw FrmwkEx(HERE, 
                "%s RW bit #%d has incorrect value", ctlMetrics[j].desc,
                ReportOffendingBitPos(value, expectedVal));
        }
    }
}


}   // namespace
