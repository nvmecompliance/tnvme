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

#include "createAndAttachMaxNamespacesAndVerify.h"
#include "globals.h"
#include "grpDefs.h"
#include "../Queues/acq.h" 
#include "../Queues/asq.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/irq.h"
#include "../Utils/queues.h"
#include "../Utils/io.h"
#include "../Singletons/informative.h"
#include "../Cmds/setFeatures.h"
#include "../Cmds/getFeatures.h"
#include "../Cmds/reservationRegister.h"
#include "../Cmds/identify.h"
#include "../Cmds/namespaceAttach.h"
#include "../Cmds/namespaceManagement.h"
#include <math.h>

typedef unsigned int uint128_t __attribute__((mode(TI)));

namespace GrpAdminNamespaceManagement {

CreateAndAttachMaxNamespacesAndVerify::CreateAndAttachMaxNamespacesAndVerify(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_12)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.2");
    mTestDesc.SetShort(     "Create CTRLR.NN namespaces and attach to all ctlrs in subsystem");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Using namespaceAttach/Management commands with SEL=0 as well as "
        "Identify with CNS values for 1.2spec tests. Create CTRLR.NN number of "
        "namespaces of equal size and attach to all controllers in subsystem");
}

CreateAndAttachMaxNamespacesAndVerify::~CreateAndAttachMaxNamespacesAndVerify()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}

CreateAndAttachMaxNamespacesAndVerify::
CreateAndAttachMaxNamespacesAndVerify(const CreateAndAttachMaxNamespacesAndVerify &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}

CreateAndAttachMaxNamespacesAndVerify &
CreateAndAttachMaxNamespacesAndVerify::operator=(const CreateAndAttachMaxNamespacesAndVerify &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}

Test::RunType
CreateAndAttachMaxNamespacesAndVerify::RunnableCoreTest(bool preserve)
{
    ///////////////////////////////////////////////////////////////////////////
    // All code contained herein must never permanently modify the state or
    // configuration of the DUT. Permanence is defined as state or configuration
    // changes that will not be restored after a cold hard reset.
    ///////////////////////////////////////////////////////////////////////////

    ConstSharedIdentifyPtr idCtrlrCap = gInformative->GetIdentifyCmdCtrlr();
    uint64_t oacs = idCtrlrCap->GetValue(IDCTRLRCAP_OACS);
    if ((oacs & OACS_SUP_NSMANAGEMENT_CMD) == 0) {
        LOG_NRM("Reporting Namespace Management not supported (oacs) 0x%02x", (uint32_t) oacs);
        return RUN_FALSE;
    }

    preserve = preserve;    // Suppress compiler error/warning
    return RUN_TRUE;        // This test is never destruct
}



void
CreateAndAttachMaxNamespacesAndVerify::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * None.
     * \endverbatim
     */
	LOG_NRM("Start CreateAndAttachMaxNamespacesAndVerify::RunCoreTest");

	SharedIOSQPtr iosq = CAST_TO_IOSQ(gRsrcMngr->GetObj(IOSQ_GROUP_ID));
	SharedIOCQPtr iocq = CAST_TO_IOCQ(gRsrcMngr->GetObj(IOCQ_GROUP_ID));
	SharedASQPtr   asq = CAST_TO_ASQ(gRsrcMngr->GetObj(ASQ_GROUP_ID));
	SharedACQPtr   acq = CAST_TO_ACQ(gRsrcMngr->GetObj(ACQ_GROUP_ID));
	//uint8_t keyToRegister[16];
	//uint32_t memAlignment = sysconf(_SC_PAGESIZE);
	//CEStat retStat;
	send_64b_bitmask prpBitmask = (send_64b_bitmask) (MASK_PRP1_PAGE | MASK_PRP2_PAGE);
	uint8_t userBuffer[4096];
    if (userBuffer == 0){} // Suppress unused variable warning

    const uint32_t *namespaceIdListBuffer32BitPtr = NULL;
    const uint16_t *controllerIdListBuffer16BitPtr;

    ConstSharedIdentifyPtr idCtrlrCap = gInformative->GetIdentifyCmdCtrlr();
    uint32_t identifyControllerMaxNSID = (uint32_t)idCtrlrCap->GetValue(IDCTRLRCAP_NN);

    // BUGBUG large assumption that NVMCAP values will not be larger than 64bit num bytes, which is 16384 PETAbytes
    // BUGBUG Also assume we need to create on GB multiples

    uint64_t identifyControllerUnallocatedCapacity = idCtrlrCap->GetValue(IDCTRLRCAP_UNVMCAP_LOWER);
    uint64_t identifyControllerTotalCapacity       = idCtrlrCap->GetValue(IDCTRLRCAP_TNVMCAP_LOWER);

    uint64_t individualNamespaceCapacityInGB       = (uint64_t) floor( identifyControllerUnallocatedCapacity / 1024.0 / 1024.0 / 1024.0 / identifyControllerMaxNSID); // In GB units
    uint64_t individualNamespaceCapacity = individualNamespaceCapacityInGB * 1024 * 1024 * 1024 / 512; // In 512B LBA units
    uint32_t newlyCreatedNSID = 0;

    if( identifyControllerUnallocatedCapacity != identifyControllerTotalCapacity) {
    	LOG_NRM("TNVMCAP != UNVMCAP, which points to a namespace being allocated but the previous test should have deleted all");
    }

	LOG_NRM("Create Identify Command To Get All Present NSIDs");
	SharedIdentifyPtr identifyCmd = SharedIdentifyPtr(new Identify());
    SharedMemBufferPtr identifyControllerStruct = SharedMemBufferPtr(new MemBuffer());
    identifyControllerStruct->Init(4096, true, 0x0);
    SharedMemBufferPtr identifyNamespaceStruct = SharedMemBufferPtr(new MemBuffer());
    identifyNamespaceStruct->Init(4096, true, 0x0);
    SharedMemBufferPtr identifyNamespaceList = SharedMemBufferPtr(new MemBuffer());
    identifyNamespaceList->Init(4096, true, 0x0);
    SharedMemBufferPtr identifyControllerList = SharedMemBufferPtr(new MemBuffer());
    identifyControllerList->Init(4096, true, 0x0);

    // Figure 86 in 1.2 spec               CNS  NSID CNTID Returns
    // CNS_Namespace                      0x00    X     -   (Identify Namespace from NSID X. If attached, receive struct, else all 0's, else invalid namespace ID)
    // CNS_Controller                     0x01    -     -   (Identify Controller struct)
    // CNS_NamespaceListAttached          0x02    Y     -   (Identify Namespace LIST, starting at NSID Y and in an increasing order)
    // CNS_NamespaceListSubsystem         0x10    Z     -   (Identify Namespace LIST, starting at NSID Z present in subsystem that may or may not be attached)
    // CNS_NamespaceStructSubsystem       0x11    X     -   (Identify Namespace from NSID A. If attached or not, receive the struct, else invalid namespace ID)
    // CNS_ControllerListAttachedToNSID   0x12    X     A   (Controller List that are attached to NSID X, starting with CNTID greater than A)
    // CNS_ControllerListSubsystem        0x13    -     B   (Controller List present in subsystem starting with CNTID greater than B)

    LOG_NRM("First ensure there are no namespaces present, as this test depends on deleteAllNamespacesAndVerify to complete successfully");
	// See 8.11 in 1.2 spec for describing the process this test is following
	identifyCmd->SetPrpBuffer(prpBitmask, identifyNamespaceList);
	identifyCmd->SetNSID( 0 );
	identifyCmd->SetCNS(  CNS_NamespaceListSubsystem ); // Get all NSIDs attached (or not) to controllers
    identifyCmd->SetCNTID(0);
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq, identifyCmd, "Reading back (up to) 1024 NSIDs attached to this controller (CNS=2). Command should pass, with 0 NSIDs to be returned", false, CESTAT_SUCCESS);
    namespaceIdListBuffer32BitPtr = (uint32_t*) identifyCmd->GetROPrpBuffer();
    if( namespaceIdListBuffer32BitPtr[0] != 0) {
    	throw FrmwkEx(HERE, "Before creating any namespaces, subsystem was expected to have no NSIDs active and present, but Identify CNS=0x10 returned at least one NSID.");
    }

    LOG_NRM("No namespaces are active, test can continue. Pull back the controller list to be used to attach all NSIDs to all controllers on subsystem");
	identifyCmd->SetPrpBuffer(prpBitmask, identifyControllerList);
	identifyCmd->SetNSID( 0 );
	identifyCmd->SetCNS(  CNS_ControllerListSubsystem ); // Get all controllers in this subsystem
	identifyCmd->SetCNTID(0);
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq, identifyCmd, "Reading back what controllers are on this subsystem (CNS=0x13), should find at LEAST one.", false, CESTAT_SUCCESS);
    controllerIdListBuffer16BitPtr = (uint16_t*) identifyCmd->GetROPrpBuffer();
    if( controllerIdListBuffer16BitPtr[0] == 0) {
    	throw FrmwkEx(HERE, "Before creating any namespaces, subsystem was expected to have at least one controller ID present, but found zero.");
    }

    SharedNamespaceAttachPtr namespaceAttachCmd = SharedNamespaceAttachPtr(new NamespaceAttach() );
    /*
    //BUGBUG APL is returning an improper list (Ctrlr IDs 1/2 wheras they ID themselves as actually 0/1)
    for(uint32_t bufferIndex = 0; bufferIndex < 4096; bufferIndex++) {
    	userBuffer[bufferIndex]=0;
    }
    userBuffer[0] = 2; // 2 Controllers
    userBuffer[4] = 1; // 1st (byte 2) is 0 and 2nd (byte 4) is 1 to match APL
    identifyControllerList->InitAlignment(4096, 4096, false, 0, userBuffer);
    LOG_NRM("Controller list hardcoded to 0/1 for the time being... BUGBUG");
    // END BUG BUG
    */
    // Set attach namespace buffer to be the full controller list, to attach all new NSIDs to to all on this subsystem
    namespaceAttachCmd->SetPrpBuffer(prpBitmask, identifyControllerList);

	LOG_NRM("Create Namespace Management to create all namespaces before attaching to all contollers");
    SharedNamespaceManagementPtr namespaceManagementCmd = SharedNamespaceManagementPtr(new NamespaceManagement() );
    SharedMemBufferPtr managementBuffer = SharedMemBufferPtr(new MemBuffer() );

    NamespaceManagementCreateStruct nscreate;
    nscreate.NSZE = individualNamespaceCapacity;
    nscreate.NCAP = individualNamespaceCapacity;
    nscreate.FLBAS = 0;
    nscreate.DPS   = 0;
    nscreate.NMIC  = 1;

    //NamespaceManagementCreateStruct nscreate( individualNamespaceCapacity, individualNamespaceCapacity, 0, 0, 1 );
    // BUGBUG we are going to assume a 512B size and barenamespace as FLBAS=0... otherwise we need to determine what format via namespace structs...
    //CreateNamespaceManagementStructure( individualNamespaceCapacity, individualNamespaceCapacity, 0, 0, 1, userBuffer);
    LOG_NRM("Each of the %d namespaces will be of 0x%llx size", identifyControllerMaxNSID, (long long unsigned int) individualNamespaceCapacity);
    nscreate.print();

    managementBuffer->InitAlignment(384, 4096, false, 0x0, (uint8_t*) &nscreate); // Contains namespace struct info for namespace management to consume (384B)
    namespaceManagementCmd->SetPrpBuffer(prpBitmask, managementBuffer);
    namespaceAttachCmd->SetSEL( 0 );
    namespaceAttachCmd->SetNSID( 0 );

	LOG_NRM("Start main loop over valid NSIDs to create/attach them.");
	for(uint64_t currentNamespaceIdToCreate = 0; currentNamespaceIdToCreate < identifyControllerMaxNSID; currentNamespaceIdToCreate++ ) {
		// Get the current
		// Create this namespace using 1/NNth amount of the total available space.
	    namespaceManagementCmd->SetSEL(0);
	    namespaceManagementCmd->SetNSID(0);
	    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq, namespaceManagementCmd, "Creating namespace that is 1/NNth size of total NVM capacity, 512B bare LBAs.", false, CESTAT_SUCCESS);
	    newlyCreatedNSID = (acq->PeekCEwithCID( namespaceManagementCmd->GetCID() ) ).t.dw0;
	    LOG_NRM("Newly created NS has NSID of 0x%llu", (long long unsigned) newlyCreatedNSID);

		// ID this namespace and see if it is all 0s. Namespace should be created but 'inactive'.
		identifyCmd->SetCNS(CNS_Namespace ); // Standard Namespace ID, should also be attached to this controller
		identifyCmd->SetNSID( newlyCreatedNSID ); //
	    identifyCmd->SetCNTID(0);
		identifyCmd->SetPrpBuffer(prpBitmask, identifyNamespaceStruct);
	    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq, identifyCmd, "Read Identify Controller struct from created NSID, expecting it to be all 0's since it has not yet attached", false, CESTAT_SUCCESS);
	    // Currently fails on FW
	    for(uint32_t bufferIndex = 0; bufferIndex < 4096; bufferIndex++) {
    		if ( identifyCmd->GetROPrpBuffer()[bufferIndex] != 0) {
    			throw FrmwkEx(HERE, "Expected Identify Namespace to returned all zero buffer as namespace should be inactive and non-attached");
    		}
    	}

	    // Now attach the drive to all controllers on the subsystem
	    namespaceAttachCmd->SetNSID( newlyCreatedNSID );
	    namespaceAttachCmd->SetSEL(0);
	    // Should already have the ControllerList associated with the NamespaceAttach command
	    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq, namespaceAttachCmd, "Attaching NSID to all controllers on subsystem", false, CESTAT_SUCCESS);

	    // Reread this NS struct back and make sure the values match what we had requested...
	    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq, identifyCmd, "Read Identify Controller struct from created NSID, expecting non-zero since it is now attached", false, CESTAT_SUCCESS);
	    // First ensure the buffer is not completely non-zero
	    bool identifyNamespaceBufferIsZeroFilled = true;
	    for(uint32_t bufferIndex = 0; bufferIndex < 4096; bufferIndex++) {
    		if ( identifyCmd->GetROPrpBuffer()[bufferIndex] != 0) {
    			identifyNamespaceBufferIsZeroFilled = false;
    			break;
    		}
    	}
	    if( identifyNamespaceBufferIsZeroFilled == true) {
	    	throw FrmwkEx(HERE, "Expected Identify Namespace to returned a non-zero buffer as namespace should now be attached");
	    }

	    // Now check a couple fields (BUGBUG should check all!)
	    IdNamespcStruct *currentNamespaceStruct = (IdNamespcStruct*) identifyCmd->GetROPrpBuffer();
	    if(currentNamespaceStruct->NSZE != individualNamespaceCapacity) {
	    	LOG_NRM("Namespace NSZE 0x%llx != what was passed to namespaceManagement 0x%08llx", (long long unsigned) currentNamespaceStruct->NSZE, (long long unsigned) individualNamespaceCapacity);
	    	throw FrmwkEx(HERE, "Newly created namespace's NSZE does not match what was sent to namespaceManagement");
	    }
	    if(currentNamespaceStruct->NCAP != individualNamespaceCapacity) {
	    	LOG_NRM("Namespace NCAP 0x%llx != what was passed to namespaceManagement 0x%08llx", (long long unsigned) currentNamespaceStruct->NCAP, (long long unsigned) individualNamespaceCapacity);
	    	throw FrmwkEx(HERE, "Newly created namespace's NCAP does not match what was sent to namespaceManagement");
	    }

	    // Repull the Identify Controller information and make sure the UNVMCAP values have decreased the amount we just allocatd for this create namespace
	    identifyCmd->SetCNS(CNS_Controller);
	    identifyCmd->SetNSID(0);
	    identifyCmd->SetCNTID(0);
	    identifyCmd->SetPrpBuffer(prpBitmask, identifyControllerStruct);
	    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq, identifyCmd, "Pull Identify Controller to check usage sizes have changed.", false, CESTAT_SUCCESS);
	    IdCtrlrCapStruct *currentControllerStruct = (IdCtrlrCapStruct*) identifyCmd->GetROPrpBuffer();
	    uint64_t expectedNVMCAP = identifyControllerTotalCapacity - (individualNamespaceCapacity * (currentNamespaceIdToCreate+1) * 512);
	    if(currentControllerStruct->UNVMCAP_LOWER != expectedNVMCAP) {
	    	LOG_NRM("Expecting UNVMCAP to decrease by last allocated namespace. UNVMCAP = 0x%llx expecting 0x%llx", (long long unsigned) currentControllerStruct->UNVMCAP_LOWER, (long long unsigned) expectedNVMCAP);
	    	throw FrmwkEx(HERE, "Identify Controller UNVMCAP is not adjusted after creating a namespace of 1/NNth size");
	    }

	    // Try to retry the attachment again and ensure a proper fail case
	    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq, namespaceAttachCmd, "Attaching an already attached NSID to all controllers again to check fail case", false, CESTAT_NS_ALREADY_ATTACHED);
	}

	// At this point we can double sanity check.
	// 1) Controller should state 0 bytes available
	// 2) Should be Cntrl.NN namespaces active and attached to all controllers on the subsystem

    LOG_NRM("Completed CreateAndAttachMaxNamespacesAndVerify::RunCoreTest");
}

}   // namespace
