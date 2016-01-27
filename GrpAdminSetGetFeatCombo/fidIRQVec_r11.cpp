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
#include "fidIRQVec_r11.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/acq.h"
#include "../Queues/asq.h"
#include "../Queues/iocq.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/irq.h"
#include "../Utils/io.h"
#include "../Utils/queues.h"
#include "../Cmds/featureDefs.h"
#include "../Cmds/getFeatures.h"
#include "../Cmds/setFeatures.h"


#define IOCQ_GROUP_ID               "IOCQ"
#define IOQ_ID                      1

namespace GrpAdminSetGetFeatCombo {


FIDIRQVec_r11::FIDIRQVec_r11(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_11),
    FIDIRQVec_r10b(grpName, testName)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.1b, section 5");
    mTestDesc.SetShort(     "Verify changes are allowed on FID = IRQ vector");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Reset ctrlr to cause a clearing of DUT state. Issue GetFeatures, "
        "FID = 0x09. Programmatically and dynamically force new, legal values "
        "into all fields of CE.DW0 and assoc changes with SetFeatures. Creates "
        "an IOCQ for each IV value and deletes after each set of feature "
        "commands (so only one IOCQ present at any time). Redo GetFeatures to "
        "verify settings were accepted.");

}


FIDIRQVec_r11::~FIDIRQVec_r11()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


FIDIRQVec_r11::
FIDIRQVec_r11(const FIDIRQVec_r11 &other) : Test(other), FIDIRQVec_r10b(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    scheme = other.scheme;
    numIrqSupport = other.numIrqSupport;
}


FIDIRQVec_r11 &
FIDIRQVec_r11::operator=(const FIDIRQVec_r11 &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    FIDIRQVec_r10b::operator=(other);
    scheme = other.scheme;
    numIrqSupport = other.numIrqSupport;
    return *this;
}


Test::RunType
FIDIRQVec_r11::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    bool capable;

    LOG_NRM("Only allowed to execute if DUT supports multiple IRQs");
    if (gCtrlrConfig->IsMSIXCapable(capable, numIrqSupport) == false)
        throw FrmwkEx(HERE, "Failed to read MSI-X capability");
    if (!capable) {
        if (gCtrlrConfig->IsMSICapable(capable, numIrqSupport) == false)
            throw FrmwkEx(HERE, "Failed to read MSI capability");
        if (!capable || numIrqSupport <= 1) {
            LOG_NRM("DUT does not support multi IRQs; unable to execute test");
            return RUN_FALSE;
        }
        else
            scheme = INT_MSI_MULTI;
    }
    else
        scheme = INT_MSIX;

    return ((preserve == true) ? RUN_FALSE : RUN_TRUE);   // Test is destructive
}


void
FIDIRQVec_r11::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * RunnableCoreTest ran prior.
     * \endverbatim
     */

    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE, "Failed to disable controller");

    LOG_NRM("Create admin queues ACQ and ASQ for test lifetime");
    SharedACQPtr acq = SharedACQPtr(new ACQ(gDutFd));
    acq->Init(5);

    SharedASQPtr asq = SharedASQPtr(new ASQ(gDutFd));
    asq->Init(5);

    uint16_t max_ivec =
        MIN(numIrqSupport, MIN((gInformative->GetFeaturesNumOfIOSQs() + 1),
        (gInformative->GetFeaturesNumOfIOCQs() + 1)));

    LOG_NRM("Setting IRQ scheme with #%d irq vectors", max_ivec);
    if (gCtrlrConfig->SetIrqScheme(scheme, max_ivec) == false) {
        throw FrmwkEx(HERE, "Unable to set IRQ scheme with num irqs #%d",
            max_ivec);
    }

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE, "Failed to enable controller");

    LOG_NRM("Create Get features and set features cmds");
    getFeaturesCmd = SharedGetFeaturesPtr(new GetFeatures());
    setFeaturesCmd = SharedSetFeaturesPtr(new SetFeatures());

    LOG_NRM("Set and Get features for IRQ vec config (FID = 0x%x)",
            FID[FID_IRQ_VEC_CONFIG]);
    getFeaturesCmd->SetFID(FID[FID_IRQ_VEC_CONFIG]);
    setFeaturesCmd->SetFID(FID[FID_IRQ_VEC_CONFIG]);

    bool ivecMismatch = false;
    for (uint8_t cd = 0; cd < 2; cd++) {
         for (uint16_t ivec = 1; ivec < max_ivec; ivec++) {
            LOG_NRM("Creating IOCQ with IVEC = %d", ivec);
            SharedIOCQPtr iocq = createQueues(asq, acq, ivec);

            LOG_NRM("Set and Get features for IVECCONF # %d", ivec);
            ivecMismatch = sendFeatures(asq, acq, cd, ivec);

            LOG_NRM("Resetting I/O queues");
            reset(iocq, asq, acq);
        }
    }

    if (ivecMismatch)
        throw FrmwkEx(HERE, "IRQ Vector Conf setting mismatched.");
}


void
FIDIRQVec_r11::reset(SharedIOCQPtr iocq, SharedASQPtr asq, SharedACQPtr acq)
{
    Queues::DeleteIOCQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iocq, asq,
        acq, "deleteIOCQ");
}


SharedIOCQPtr
FIDIRQVec_r11::createQueues(SharedASQPtr asq, SharedACQPtr acq, uint16_t ivec)
{
    uint32_t numEntriesIOQ = 2; // minimum

    gCtrlrConfig->SetIOCQES(gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_CQES) & 0xf);
    return Queues::CreateIOCQContigToHdw(mGrpName, mTestName,
        CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, numEntriesIOQ, false,
        IOCQ_GROUP_ID, true, ivec);
}


}   // namespace
