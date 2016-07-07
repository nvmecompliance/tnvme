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

#include "unsupportRsvdFields_r12.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/io.h"
#include "../Cmds/flush.h"
#include <math.h>


#define NAMSPC_LIST_SIZE          1024 // N
#define NAMSPC_ENTRY_SIZE         4    // bytes


namespace GrpNVMFlushCmd {


UnsupportRsvdFields_r12::UnsupportRsvdFields_r12(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_12)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.2, section 6.8");
    mTestDesc.SetShort(     "Set unsupported/rsvd fields in cmd");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Unsupported DW's and rsvd fields are treated identical, the recipient "
        "is not required to check their value. Receipt of reserved coded "
        "values shall be reported as an error. Determine Identify.NN and issue "
        "flush cmd to all namspc, expect success. Then issue same cmd setting "
        "all unsupported/rsvd fields, expect success. Set: DW0_b15:10, DW2, "
        "DW3, DW4, DW5, DW6, DW7, DW8, DW9, DW10, DW11, DW12, DW13, DW14, "
        "DW15.");
}


UnsupportRsvdFields_r12::~UnsupportRsvdFields_r12()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}


UnsupportRsvdFields_r12::
UnsupportRsvdFields_r12(const UnsupportRsvdFields_r12 &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}


UnsupportRsvdFields_r12 &
UnsupportRsvdFields_r12::operator=(const UnsupportRsvdFields_r12 &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}


Test::RunType
UnsupportRsvdFields_r12::RunnableCoreTest(bool preserve)
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
UnsupportRsvdFields_r12::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * 1) Test CreateResources_r10b has run prior.
     * \endverbatim
     */
    CE ce;
    vector<uint32_t> activeNamespaces;
    string work;
    LOG_NRM("Admin queue setup");

    SharedASQPtr asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID));
    SharedACQPtr acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID));
    LOG_NRM("Test proper");


    LOG_NRM("Form identify cmd for namespace list and associate some buffer");
    SharedIdentifyPtr idCmdNamSpcList = SharedIdentifyPtr(new Identify());
    idCmdNamSpcList->SetCNS(CNS_NamespaceListActive);
    idCmdNamSpcList->SetNSID(0);

    SharedMemBufferPtr idMemNamSpcList = SharedMemBufferPtr(new MemBuffer());
    idMemNamSpcList->InitAlignment(NAMSPC_LIST_SIZE * NAMSPC_ENTRY_SIZE);

    send_64b_bitmask idPrpNamSpc =
        (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    idCmdNamSpcList->SetPrpBuffer(idPrpNamSpc, idMemNamSpcList);

    LOG_NRM("Sending Identify command CNS.%02Xh", CNS_NamespaceListActive);
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq,
        idCmdNamSpcList, "namspcList", true);

    LOG_NRM("Reading in active Namespaces.");
    const uint8_t *data = &((idCmdNamSpcList->GetROPrpBuffer())[0]);

    while ((*((uint32_t *)data) & 0xffff) != 0x0000){
        uint32_t nsid = (*((uint32_t *)data) & 0xffff);
        LOG_NRM("Found active NSID: %08X", nsid);
        activeNamespaces.push_back(nsid);
        data += 4;
    }

    // Lookup objs which were created in a prior test within group
    SharedIOSQPtr iosq = CAST_TO_IOSQ(gRsrcMngr->GetObj(IOSQ_GROUP_ID));
    SharedIOCQPtr iocq = CAST_TO_IOCQ(gRsrcMngr->GetObj(IOCQ_GROUP_ID));

    ConstSharedIdentifyPtr idCtrlr = gInformative->GetIdentifyCmdCtrlr();

    for (size_t i = 0; i < activeNamespaces.size(); i++) {
        LOG_NRM("Processing namspc %ld", i);
        SharedFlushPtr flushCmd = SharedFlushPtr(new Flush());
        flushCmd->SetNSID(activeNamespaces[i]);

        ce = IO::SendAndReapCmdWhole(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq,
            flushCmd, "none.set", true);
        if (ce.n.SF.b.SC != 0x0 || ce.n.SF.b.SCT != 0x0){
            throw new FrmwkEx(HERE, "Error sending flush command expected (SCT:SC) 0x00:0x00, "
                    "but detected 0x%02X:0x%02X", ce.n.SF.b.SCT, ce.n.SF.b.SC);
        }

        if (ce.n.reserved != 0){
            throw new FrmwkEx(HERE, "Reserved completion entry not cleared found: %08X",
                    ce.n.reserved);
        } else if(ce.n.cmdSpec != 0){
            throw new FrmwkEx(HERE, "Command specific field not cleared found: %08X",
                    ce.n.cmdSpec);
        }

        LOG_NRM("Set all cmd's rsvd bits");
        uint32_t work = flushCmd->GetDword(0);
        work |= 0x00003c00;      // Set DW0_b13:10 bits
        flushCmd->SetDword(work, 0);

        flushCmd->SetDword(0xffffffff, 2);
        flushCmd->SetDword(0xffffffff, 3);
        flushCmd->SetDword(0xffffffff, 4);
        flushCmd->SetDword(0xffffffff, 5);
        flushCmd->SetDword(0xffffffff, 6);
        flushCmd->SetDword(0xffffffff, 7);
        flushCmd->SetDword(0xffffffff, 8);
        flushCmd->SetDword(0xffffffff, 9);
        flushCmd->SetDword(0xffffffff, 10);
        flushCmd->SetDword(0xffffffff, 11);
        flushCmd->SetDword(0xffffffff, 12);
        flushCmd->SetDword(0xffffffff, 13);
        flushCmd->SetDword(0xffffffff, 14);
        flushCmd->SetDword(0xffffffff, 15);

        ce = IO::SendAndReapCmdWhole(mGrpName, mTestName, CALC_TIMEOUT_ms(1),
                iosq, iocq,flushCmd, "all.set", true);
        if (ce.n.SF.b.SC != 0x0 || ce.n.SF.b.SCT != 0x0){
            throw new FrmwkEx(HERE, "Error sending flush command expected (SCT:SC) 0x00:0x00, "
                    "but detected 0x%02X:0x%02X", ce.n.SF.b.SCT, ce.n.SF.b.SC);
        }

        if (ce.n.reserved != 0){
            throw new FrmwkEx(HERE, "Reserved completion entry not cleared found: %08X",
                    ce.n.reserved);
        } else if(ce.n.cmdSpec != 0){
            throw new FrmwkEx(HERE, "Command specific field not cleared found: %08X",
                    ce.n.cmdSpec);
        }
    }

    // This highestNSID could provide a case where it issues to an inactive NSID
    // In the case of an inactive NSID the error status would be Invalid Field
    /*uint32_t highestNSID = 0;
    for (uint32_t i = 0; i < activeNamespaces.size(); i++){
        if (activeNamespaces[i] > highestNSID)
            highestNSID = activeNamespaces[i];
    }

    if (highestNSID != 0xffffffff){
        SharedFlushPtr invalidFlushCmd = SharedFlushPtr(new Flush());
        invalidFlushCmd->SetNSID(highestNSID + 1);
        // Could be Invalid Field or Invalid Namespace or Format
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq, iocq,
            invalidFlushCmd, "none.set", true, CESTAT_INVAL_NAMSPC);
    }*/
    ConstSharedIdentifyPtr idCtrlrStruct = gInformative->GetIdentifyCmdCtrlr();
    uint32_t nn = (uint32_t)idCtrlrStruct->GetValue(IDCTRLRCAP_NN);
    // Issuing flush cmd with a few invalid NSIDs
    int count = 0;
    for (uint64_t i = 0; pow(2, i) <= 0xffffffff; i++) {
        if (pow(2, i) <= nn)
            continue;
        LOG_NRM("Issue flush cmd with illegal namspc ID=%llu",
                (unsigned long long)i);
        SharedFlushPtr invalidFlushCmd = SharedFlushPtr(new Flush());
        invalidFlushCmd->SetNSID(pow(2, i));
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), iosq,
                iocq, invalidFlushCmd, "Flush.Invalid", true, CESTAT_INVAL_NAMSPC);
        count++;
    }
    LOG_NRM("Ran with: %d invalid NSIDs", count);
}


}   // namespace

