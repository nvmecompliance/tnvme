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

#include <initializer_list>
#include <boost/format.hpp>
#include "maxQSizeExceed_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/queues.h"
#include "../Utils/io.h"
#include "../Utils/irq.h"

#define     MAX_Q_ENTRIES       0x10000  // 1- based value.

namespace GrpAdminCreateIOCQCmd {


MaxQSizeExceed_r10b::MaxQSizeExceed_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Issue CreateIOCQ cause SC = Maximum queue size exceeded");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "CreateIOCQ cmds specifying DW10.QSIZE ranging from (CAP.MQES + 2) "
        "to 0xffff, expect failure.");
}


MaxQSizeExceed_r10b::~MaxQSizeExceed_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


MaxQSizeExceed_r10b::
MaxQSizeExceed_r10b(const MaxQSizeExceed_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


MaxQSizeExceed_r10b &
MaxQSizeExceed_r10b::operator=(const MaxQSizeExceed_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
MaxQSizeExceed_r10b::RunnableCoreTest(bool preserve)
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
MaxQSizeExceed_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * None.
     * \endverbatim
     */
    string work;
    bool enableLog;

    uint64_t ctrlCapReg;
    LOG_NRM("Determine the max IOQ entries supported");
    if (gRegisters->Read(CTLSPC_CAP, ctrlCapReg) == false)
        throw FrmwkEx(HERE, "Unable to determine MQES");
    uint32_t maxIOQEntries = (ctrlCapReg & CAP_MQES);
    maxIOQEntries += 1;      // convert to 1 - based.

    if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Create admin queues ACQ and ASQ");
    SharedACQPtr acq = SharedACQPtr(new ACQ(gDutFd));
    acq->Init(5);

    SharedASQPtr asq = SharedASQPtr(new ASQ(gDutFd));
    asq->Init(5);

    // All queues will use identical IRQ vector
    IRQ::SetAnySchemeSpecifyNum(1);

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);

    LOG_NRM("Setup element size for the IOCQ");
    gCtrlrConfig->SetIOCQES(gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_CQES) & 0xf);

    LOG_NRM("Create IOCQ with illegal Q entries ranging from 0x%04X to 0x10000",
        maxIOQEntries + 1);
    list<uint32_t> illegalQSizes = GetIllegalQSizes(maxIOQEntries);
    for (list<uint32_t>::iterator qSize = illegalQSizes.begin();
        qSize != illegalQSizes.end(); qSize++) {
        LOG_NRM("Process CreateIOCQ Cmd with qSize #0x%04X", *qSize);
        SharedIOCQPtr iocq = SharedIOCQPtr(new IOCQ(gDutFd));
        iocq->Init(IOQ_ID, *qSize, true, 0);
        SharedCreateIOCQPtr createIOCQCmd =
            SharedCreateIOCQPtr(new CreateIOCQ());
        createIOCQCmd->Init(iocq);

        work = str(boost::format("qSize.%04Xh") % *qSize);
        enableLog = false;
        if ((*qSize <= (maxIOQEntries + 8)) || (*qSize >= (0xFFFF - 8)))
            enableLog = true;

        LOG_NRM("Send n reap ACQ for CreateIOCQCmd; qSize #0x%04X", *qSize);
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
            createIOCQCmd, work, enableLog, CESTAT_MAX_Q_SIZE_EXCEED);
    }
}


list<uint32_t>
MaxQSizeExceed_r10b::GetIllegalQSizes(uint32_t maxIOQEntries)
{
    list<uint32_t> illegalQSizes;
    for (uint32_t qSize = (maxIOQEntries + 2); qSize <= (MAX_Q_ENTRIES + 1);
        qSize <<= 1) {
        if (qSize < MAX_Q_ENTRIES) {
            illegalQSizes.push_back(qSize - 1);
            illegalQSizes.push_back(qSize);
            illegalQSizes.push_back(qSize + 1);
        }
    }
    if (maxIOQEntries < MAX_Q_ENTRIES) {
        illegalQSizes.remove(MAX_Q_ENTRIES);
        illegalQSizes.push_back(MAX_Q_ENTRIES);
    }
    return illegalQSizes;
}


}   // namespace

