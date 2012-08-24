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
#include "maxQSizeExceed_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Utils/queues.h"
#include "../Utils/io.h"

#define     MAX_Q_ENTRIES       0x10000  // 1- based value.

namespace GrpAdminCreateIOSQCmd {


MaxQSizeExceed_r10b::MaxQSizeExceed_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Issue CreateIOSQ cause SC=Maximum queue size exceeded");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Issue a CreateIOCQ cmd, with QID=1, num elements=2, expect success. "
        "Then issue a correlating CreateIOSQ cmds specifying DW10.QSIZE "
        "ranging from (CAP.MQES+2) to 0xffff, expect failure. Then issue "
        "CreateIOSQ with the last legal Q size, expect success");
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
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */
    uint64_t mqes;
    string qualify;
    uint64_t maxIOQEntries = 2;

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    LOG_NRM("Setup element sizes for the IOQ's");
    uint8_t iocqes = (gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_CQES) & 0xf);
    uint8_t iosqes = (gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_SQES) & 0xf);
    gCtrlrConfig->SetIOCQES(iocqes);
    gCtrlrConfig->SetIOSQES(iosqes);

    SharedIOCQPtr iocq = Queues::CreateIOCQContigToHdw(mGrpName, mTestName,
        CALC_TIMEOUT_ms(1), asq, acq, IOQ_ID, maxIOQEntries, false,
        IOCQ_GROUP_ID, false, 0);

    if (gRegisters->Read(CTLSPC_CAP, mqes) == false)
        throw FrmwkEx(HERE, "Unable to determine CAP.MQES");
    mqes &= CAP_MQES;
    mqes = mqes + 1; // convert to 1 - based
    LOG_NRM("DUT's largest supported IOSQ size = 0x%05lX", mqes);

    list<uint32_t> illegalQSizes = GetIllegalQSizes(mqes);
    for (list<uint32_t>::iterator qSize = illegalQSizes.begin();
        qSize != illegalQSizes.end(); qSize++) {
        LOG_NRM("Create an IOSQ object with test lifetime");
        SharedIOSQPtr iosq = SharedIOSQPtr(new IOSQ(gDutFd));

        LOG_NRM("Issue CreateIOSQ with illegal Q size = 0x%08X", *qSize);
        iosq->Init(IOQ_ID, *qSize, IOQ_ID, 0);

        LOG_NRM("Form a Create IOSQ cmd to perform queue creation");
        SharedCreateIOSQPtr createIOSQCmd =
            SharedCreateIOSQPtr(new CreateIOSQ());
        createIOSQCmd->Init(iosq);

        qualify = str(boost::format("qsize.0x%04X") % *qSize);
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
            createIOSQCmd, qualify, false, CESTAT_MAX_Q_SIZE_EXCEED);
    }

    LOG_NRM("Issue largest legal sized IOSQ  0x%05lX, should succeed", mqes);

    LOG_NRM("Create an IOSQ object with test lifetime");
    SharedIOSQPtr iosq = SharedIOSQPtr(new IOSQ(gDutFd));

    LOG_NRM("Issue CreateIOSQ with legal Q size = 0x%08lX", mqes);
    iosq->Init(IOQ_ID, mqes, IOQ_ID, 0);

    LOG_NRM("Form a Create IOSQ cmd to perform queue creation");
    SharedCreateIOSQPtr createIOSQCmd =
        SharedCreateIOSQPtr(new CreateIOSQ());
    createIOSQCmd->Init(iosq);

    qualify = str(boost::format("qsize.0x%04lX") % mqes);
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
        createIOSQCmd, qualify, false);

    Queues::DeleteIOSQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq,
        asq, acq, "", false);
    Queues::DeleteIOCQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iocq,
        asq, acq, "", false);
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

