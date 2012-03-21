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

#include "invalidMSIXIRQ_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "createResources_r10b.h"
#include "../Queues/iocq.h"
#include "../Cmds/createIOCQ.h"
#include "../Utils/io.h"

#define IOQ_ID                      1

namespace GrpInterrupts {

// todo static uint32_t NumEntriesIOQ =     2;


InvalidMSIXIRQ_r10b::InvalidMSIXIRQ_r10b(int fd, string grpName, string testName,
    ErrorRegs errRegs) :
    Test(fd, grpName, testName, SPECREV_10b, errRegs)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 4,7");
    mTestDesc.SetShort(     "Assoc illegal MSI-X IRQ to cause SC=Invalid interrupt vector");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Only allowed to execute test if the DUT supports MSI-X IRQ's by "
        "reporting the MSIXCAP PCI structures. Determine X, the maximum IRQ "
        "vector supported by the DUT by referencing MSIXCAP.MXC.TS. Issue "
        "multiple CreateIOCQ cmds, num elements=2, QID=1, enable IRQ's, and "
        "set DW11.IV by looping from (X+1) to 2047, expect failure.");
}


InvalidMSIXIRQ_r10b::~InvalidMSIXIRQ_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


InvalidMSIXIRQ_r10b::
InvalidMSIXIRQ_r10b(const InvalidMSIXIRQ_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


InvalidMSIXIRQ_r10b &
InvalidMSIXIRQ_r10b::operator=(const InvalidMSIXIRQ_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


void
InvalidMSIXIRQ_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) This is the 1st within GrpBasicInit.
     * 2) An individual test within this group cannot run, the entire group
     *    must be executed every time. Each subsequent test relies on the prior.
     * \endverbatim
     */
/* todo not quite ready yet
    bool capable;
    char work[256];
    uint16_t numIrqSupport;


    // Only allowed to execute if DUT supports MSI-X IRQ's
    if (gCtrlrConfig->IsMSIXCapable(capable, numIrqSupport) == false)
        throw FrmwkEx();
    else if (capable == false) {
        LOG_NRM("DUT does not support MSI-X IRQ's; unable to execute test");
        return;
    }

    if (gCtrlrConfig->SetState(ST_DISABLE) == false)
        throw FrmwkEx();

    // dnvme must be able to use the max number IRQs, however DUT will reject
    if (gCtrlrConfig->SetIrqScheme(INT_MSIX,
        CtrlrConfig::MAX_MSIX_IRQ_VEC + 1) == false) {

        throw FrmwkEx();
    }

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx();

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    LOG_NRM("Create an IOCQ object with test lifetime");
    gCtrlrConfig->SetIOCQES(gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_CQES) & 0xf);

    LOG_NRM("Last suppport MSIX IRQ vec = %d", (numIrqSupport - 1));
    for (int i = numIrqSupport; i <= CtrlrConfig::MAX_MSIX_IRQ_VEC; i++) {
        LOG_NRM("Attempt to utilize illegal IRQ vec %d", i);
        SharedIOCQPtr iocq = SharedIOCQPtr(new IOCQ(mFd));

        LOG_NRM("Allocate contiguous memory; IOCQ has ID=%d", IOQ_ID);
        iocq->Init(IOQ_ID, NumEntriesIOQ, true, i);

        LOG_NRM("Form a Create IOCQ cmd to perform queue creation");
        SharedCreateIOCQPtr createIOCQCmd = SharedCreateIOCQPtr(
            new CreateIOCQ());
        createIOCQCmd->Init(iocq);

        snprintf(work, sizeof(work), "irq%d", i);
        IO::SendCmdToHdw(mGrpName, mTestName, DEFAULT_CMD_WAIT_ms, asq, acq,
            createIOCQCmd, work, true, CESTAT_INVAL_INT_VEC);
    }
 todo not quite ready yet */
}

}   // namespace
