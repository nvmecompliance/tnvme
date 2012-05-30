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

namespace GrpAdminCreateIOSQCmd {


MaxQSizeExceed_r10b::MaxQSizeExceed_r10b(int fd, string mGrpName,
    string mTestName, ErrorRegs errRegs) :
    Test(fd, mGrpName, mTestName, SPECREV_10b, errRegs)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 6");
    mTestDesc.SetShort(     "Issue CreateIOSQ cause SC=Maximum queue size exceeded");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Issue a CreateIOCQ cmd, with QID=1, num elements=2, expect success. "
        "Then issue a correlating CreateIOSQ cmds specifying DW10.QSIZE "
        "ranging from (CAP.MQES+2) to 0xffff, expect failure. Then issue "
        " CreateIOSQ with the last legal Q size, expect success");
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
        DEFAULT_CMD_WAIT_ms, asq, acq, IOQ_ID, maxIOQEntries, false,
        IOCQ_GROUP_ID, false, 0);

    if (gRegisters->Read(CTLSPC_CAP, mqes) == false)
        throw FrmwkEx(HERE, "Unable to determine CAP.MQES");
    mqes &= CAP_MQES;
    LOG_NRM("DUT's largest supported IOSQ size = 0x%05lX", (mqes + 1));

    for (uint32_t i = (mqes + 2); i <= 0xffff; i++) {
        LOG_NRM("Create an IOSQ object with test lifetime");
        SharedIOSQPtr iosq = SharedIOSQPtr(new IOSQ(gDutFd));

        LOG_NRM("Issue CreateIOSQ with illegal Q size = 0x%08X", i);
        iosq->Init(IOQ_ID, i, IOQ_ID, 0);

        LOG_NRM("Form a Create IOSQ cmd to perform queue creation");
        SharedCreateIOSQPtr createIOSQCmd =
            SharedCreateIOSQPtr(new CreateIOSQ());
        createIOSQCmd->Init(iosq);

        qualify = str(boost::format("qsize.0x%04X") % i);
        IO::SendAndReapCmd(mGrpName, mTestName, DEFAULT_CMD_WAIT_ms, asq, acq,
            createIOSQCmd, qualify, false, CESTAT_MAX_Q_SIZE_EXCEED);
    }


    uint32_t i = (mqes + 1);
    LOG_NRM("Issue largest legal sized IOSQ  0x%05X, should succeed", i);

    LOG_NRM("Create an IOSQ object with test lifetime");
    SharedIOSQPtr iosq = SharedIOSQPtr(new IOSQ(gDutFd));

    LOG_NRM("Issue CreateIOSQ with legal Q size = 0x%08X", i);
    iosq->Init(IOQ_ID, i, IOQ_ID, 0);

    LOG_NRM("Form a Create IOSQ cmd to perform queue creation");
    SharedCreateIOSQPtr createIOSQCmd =
        SharedCreateIOSQPtr(new CreateIOSQ());
    createIOSQCmd->Init(iosq);

    qualify = str(boost::format("qsize.0x%04X") % i);
    IO::SendAndReapCmd(mGrpName, mTestName, DEFAULT_CMD_WAIT_ms, asq, acq,
        createIOSQCmd, qualify, false);

    Queues::DeleteIOSQToHdw(mGrpName, mTestName, DEFAULT_CMD_WAIT_ms, iosq,
        asq, acq, "", false);
    Queues::DeleteIOCQToHdw(mGrpName, mTestName, DEFAULT_CMD_WAIT_ms, iocq,
        asq, acq, "", false);
}


}   // namespace

