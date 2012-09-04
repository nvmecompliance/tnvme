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

#include "deleteIOQContig_r10b.h"
#include "globals.h"
#include "createACQASQ_r10b.h"
#include "createIOQContigPoll_r10b.h"
#include "grpDefs.h"
#include "../Utils/queues.h"
#include "../Utils/kernelAPI.h"

namespace GrpBasicInit {


DeleteIOQContig_r10b::DeleteIOQContig_r10b(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_10b)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 7");
    mTestDesc.SetShort(     "Delete contiguous IOCQ and IOSQ's");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Issue the admin commands Delete I/O SQ and Delete I/Q CQ"
        "to the ASQ and reap the resulting CE's from the ACQ to certify "
        "those the contiguous IOQ's have been deleted. Dumping driver "
        "metrics before and after the deletion will prove the dnvme/hdw has "
        "removed those Q's");
}


DeleteIOQContig_r10b::~DeleteIOQContig_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


DeleteIOQContig_r10b::
DeleteIOQContig_r10b(const DeleteIOQContig_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


DeleteIOQContig_r10b &
DeleteIOQContig_r10b::operator=(const DeleteIOQContig_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
DeleteIOQContig_r10b::RunnableCoreTest(bool preserve)
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
DeleteIOQContig_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Both test CreateIOQContigPoll_r10b & CreateIOQDiscontigPoll_r10b has
     *    run prior, or Both test CreateIOQContigIrq_r10b &
     *    CreateIOQDiscontigIrq_r10b has run prior
     * 2) An individual test within this group cannot run, the entire group
     *    must be executed every time. Each subsequent test relies on the prior.
     * \endverbatim
     */

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))


    // According to spec, if one deletes the CQ before the SQ it's a "shall not"
    // statement which means it will have undefined behavior and thus there is
    // nothing to gain by attempting such action.
    LOG_NRM("Lookup IOSQ which was created in a prior test within group");
    SharedIOSQPtr iosq = CAST_TO_IOSQ(gRsrcMngr->GetObj(IOSQ_CONTIG_GROUP_ID))
    Queues::DeleteIOSQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
        iosq, asq, acq);


    LOG_NRM("Lookup IOCQ which was created in a prior test within group");
    SharedIOCQPtr iocq = CAST_TO_IOCQ(gRsrcMngr->GetObj(IOCQ_CONTIG_GROUP_ID))
    Queues::DeleteIOCQToHdw(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
        iocq, asq, acq);
}

}   // namespace

