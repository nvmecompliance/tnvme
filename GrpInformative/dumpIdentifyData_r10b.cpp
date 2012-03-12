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

#include <unistd.h>
#include "grpDefs.h"
#include "globals.h"
#include "dumpIdentifyData_r10b.h"
#include "../Cmds/identify.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/queues.h"
#include "../Utils/io.h"

namespace GrpInformative {


DumpIdentifyData_r10b::DumpIdentifyData_r10b(int fd, string grpName,
    string testName, ErrorRegs errRegs) :
    Test(fd, grpName, testName, SPECREV_10b, errRegs)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.0b, section 7");
    mTestDesc.SetShort(     "Issue the identify cmd");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Issue the identify cmd to the ASQ. Request both controller and all "
        "namespace pages. Dump this data to the log directory");
}


DumpIdentifyData_r10b::~DumpIdentifyData_r10b()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


DumpIdentifyData_r10b::
DumpIdentifyData_r10b(const DumpIdentifyData_r10b &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


DumpIdentifyData_r10b &
DumpIdentifyData_r10b::operator=(const DumpIdentifyData_r10b &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


void
DumpIdentifyData_r10b::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     *  \endverbatim
     */
    uint32_t isrCount;

    // Lookup objs which were created in a prior test within group
    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID))
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID))

    // Assuming the cmd we issue will result in only a single CE
    if (acq->ReapInquiry(isrCount, true) != 0) {
        acq->Dump(
            FileSystem::PrepLogFile(mGrpName, mTestName, "acq",
            "notEmpty"), "Test assumption have not been met");
        throw FrmwkEx(
            "The ACQ should not have any CE's waiting before testing");
    }

    SendIdentifyCtrlrStruct(asq, acq);
    SendIdentifyNamespaceStruct(asq, acq);
}


void
DumpIdentifyData_r10b::SendIdentifyCtrlrStruct(SharedASQPtr asq,
    SharedACQPtr acq)
{
    LOG_NRM("Create 1st identify cmd and assoc some buffer memory");
    SharedIdentifyPtr idCmdCtrlr = SharedIdentifyPtr(new Identify());
    LOG_NRM("Force identify to request ctrlr capabilities struct");
    idCmdCtrlr->SetCNS(true);
    SharedMemBufferPtr idMemCap = SharedMemBufferPtr(new MemBuffer());
    idMemCap->InitAlignment(Identify::IDEAL_DATA_SIZE, sizeof(uint64_t),
        true, 0);
    send_64b_bitmask prpReq =
        (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    idCmdCtrlr->SetPrpBuffer(prpReq, idMemCap);

    IO::SendCmdToHdw(mGrpName, mTestName, DEFAULT_CMD_WAIT_ms, asq, acq,
        idCmdCtrlr, "IdCtrlStruct", true);

    // Update the Informative singleton for all tests to see and use
    gInformative->SetIdentifyCmdCtrlr(idCmdCtrlr);
}


void
DumpIdentifyData_r10b::SendIdentifyNamespaceStruct(SharedASQPtr asq,
    SharedACQPtr acq)
{
    uint64_t numNamSpc;
    char qualifier[20];


    ConstSharedIdentifyPtr idCmdCtrlr = gInformative->GetIdentifyCmdCtrlr();
    if ((numNamSpc = idCmdCtrlr->GetValue(IDCTRLRCAP_NN)) == 0)
        throw FrmwkEx("Required to support >= 1 namespace");

    LOG_NRM("Gather %lld identify namspc structs from DUT",
        (unsigned long long)numNamSpc);
    for (uint64_t namSpc = 1; namSpc <= numNamSpc; namSpc++) {
        snprintf(qualifier, sizeof(qualifier), "idCmdNamSpc-%llu",
            (long long unsigned int)namSpc);

        LOG_NRM("Create identify cmd #%llu & assoc some buffer memory",
            (long long unsigned int)namSpc);
        SharedIdentifyPtr idCmdNamSpc = SharedIdentifyPtr(new Identify());
        LOG_NRM("Force identify to request namespace struct #%llu",
            (long long unsigned int)namSpc);
        idCmdNamSpc->SetCNS(false);
        idCmdNamSpc->SetNSID(namSpc);
        SharedMemBufferPtr idMemNamSpc = SharedMemBufferPtr(new MemBuffer());
        idMemNamSpc->InitAlignment(Identify::IDEAL_DATA_SIZE, sizeof(uint64_t),
            true, 0);
        send_64b_bitmask idPrpNamSpc =
            (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
        idCmdNamSpc->SetPrpBuffer(idPrpNamSpc, idMemNamSpc);

        IO::SendCmdToHdw(mGrpName, mTestName, DEFAULT_CMD_WAIT_ms, asq, acq,
            idCmdNamSpc, qualifier, true);

        gInformative->SetIdentifyCmdNamespace(idCmdNamSpc);
    }
}

}   // namespace
