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
#include "invalidQID_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/queues.h"
#include "../Utils/io.h"
#include "../Utils/irq.h"

namespace GrpAdminCreateIOCQCmd {


InvalidQID_r10b::InvalidQID_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Issue CreateIOCQ cause SC = Invalid queue identifier");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Calc X, where X = max num IOCQ's DUT will support. Issue CreateIOCQ "
        "cmds specifying QID's ranging from (X+1) to 0xffff, expect failure.");
}


InvalidQID_r10b::~InvalidQID_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


InvalidQID_r10b::
InvalidQID_r10b(const InvalidQID_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


InvalidQID_r10b &
InvalidQID_r10b::operator=(const InvalidQID_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
InvalidQID_r10b::RunnableCoreTest(bool preserve)
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
InvalidQID_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * None.
     * \endverbatim
     */
    string work;
    bool enableLog;
    uint64_t maxIOQEntries = 2;

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
    gCtrlrConfig->SetIOCQES(CtrlrConfig::MIN_IOCQES);
    gCtrlrConfig->SetIOSQES(CtrlrConfig::MIN_IOSQES);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw FrmwkEx(HERE);

    // Calc X, max no. of IOCQ's DUT supports.
    uint32_t X = gInformative->GetFeaturesNumOfIOCQs();
    LOG_NRM("Maximum num of IOCQ's DUT will support = %d", X);

    LOG_NRM("Setup element size for the IOCQ");
    gCtrlrConfig->SetIOCQES(gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_CQES) & 0xf);

    LOG_NRM("Issue CreateIOCQ cmds with QID's ranging from %d to %d",
        (X + 1), MAX_IOQ_ID);

    list<uint32_t> illegalQIDs = GetIllegalQIDs(X + 1);
    for (list<uint32_t>::iterator qId = illegalQIDs.begin();
        qId != illegalQIDs.end(); qId++) {
        LOG_NRM("Process each CreateIOCQCmd with iocq id #%d", *qId);
        SharedIOCQPtr iocq = SharedIOCQPtr(new IOCQ(gDutFd));
        iocq->Init(*qId, maxIOQEntries, true, 0);
        SharedCreateIOCQPtr createIOCQCmd =
            SharedCreateIOCQPtr(new CreateIOCQ());
        createIOCQCmd->Init(iocq);

        work = str(boost::format("iocqId.%d") % *qId);
        enableLog = false;
        if ((*qId <= (X + 8)) || (*qId >= (MAX_IOQ_ID - 8)))
            enableLog = true;

        LOG_NRM("Send and reap cmd with CQ ID #%d", *qId);
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
            createIOCQCmd, work, enableLog, CESTAT_INVALID_QID);
    }
}


list<uint32_t>
InvalidQID_r10b::GetIllegalQIDs(uint32_t maxQIdsSupported)
{
    list<uint32_t> illegalQIDs;
    for (uint32_t qId = (maxQIdsSupported + 1); qId <= (MAX_IOQ_ID + 1);
        qId <<= 1) {
        if (qId < MAX_IOQ_ID) {
            illegalQIDs.push_back(qId - 1);
            illegalQIDs.push_back(qId);
            illegalQIDs.push_back(qId + 1);
        }
    }
    if (maxQIdsSupported < MAX_IOQ_ID) {
        illegalQIDs.remove(MAX_IOQ_ID - 1);
        illegalQIDs.remove(MAX_IOQ_ID);
        illegalQIDs.push_back(MAX_IOQ_ID - 1);
        illegalQIDs.push_back(MAX_IOQ_ID);
    }
    return illegalQIDs;
}


}   // namespace

