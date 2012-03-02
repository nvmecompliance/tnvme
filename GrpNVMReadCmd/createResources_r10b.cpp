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

#include "createResources_r10b.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/acq.h"
#include "../Queues/asq.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/queues.h"
#include "../Utils/irq.h"

namespace GrpNVMReadCmd {

static uint16_t NumEntriesIOQ =     2;


CreateResources_r10b::CreateResources_r10b(int fd, string grpName, string testName,
    ErrorRegs errRegs) :
    Test(fd, grpName, testName, SPECREV_10b, errRegs)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 5");
    mTestDesc.SetShort(     "Create resources needed by subsequent tests");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Create resources with group lifetime which are needed by subsequent "
        "tests");
}


CreateResources_r10b::~CreateResources_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


CreateResources_r10b::
CreateResources_r10b(const CreateResources_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


CreateResources_r10b &
CreateResources_r10b::operator=(const CreateResources_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


bool
CreateResources_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) This is the 1st within GrpBasicInit.
     * \endverbatim
     */
    SharedACQPtr acq = CAST_TO_ACQ(
        gRsrcMngr->AllocObj(Trackable::OBJ_ACQ, ACQ_GROUP_ID))
    acq->Init(5);

    SharedASQPtr asq = CAST_TO_ASQ(
        gRsrcMngr->AllocObj(Trackable::OBJ_ASQ, ASQ_GROUP_ID))
    asq->Init(5);

    // All queues will use identical IRQ vector
    IRQ::SetAnySchemeSpecifyNum(1);     // throws upon error

    gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
    if (gCtrlrConfig->SetState(ST_ENABLE) == false)
        throw exception();

    gCtrlrConfig->SetIOCQES(IOCQ::COMMON_ELEMENT_SIZE_PWR_OF_2);
    Queues::CreateIOCQContigToHdw(mFd, mGrpName, mTestName, DEFAULT_CMD_WAIT_ms,
        asq, acq, IOQ_ID, NumEntriesIOQ, true, IOCQ_CONTIG_GROUP_ID, true, 0);

    gCtrlrConfig->SetIOSQES(IOSQ::COMMON_ELEMENT_SIZE_PWR_OF_2);
    Queues::CreateIOSQContigToHdw(mFd, mGrpName, mTestName, DEFAULT_CMD_WAIT_ms,
        asq, acq, IOQ_ID, NumEntriesIOQ, true, IOSQ_CONTIG_GROUP_ID, IOQ_ID, 0);

    return true;
}

}   // namespace
