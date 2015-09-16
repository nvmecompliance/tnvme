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

#include "deleteAllNamespacesAndVerify.h"
#include "namespaceManagementUtilities.h"
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

namespace GrpAdminNamespaceManagement {

DeleteAllNamespacesAndVerify::DeleteAllNamespacesAndVerify(
    string grpName, string testName) :
    Test(grpName, testName, SPECREV_12)
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mTestDesc.SetCompliance("revision 1.2");
    mTestDesc.SetShort(     "Delete all current namespaces and verify no longer attached");
    // No string size limit for the long description
    mTestDesc.SetLong(
        "Using namespaceAttach/Management commands with SEL=1 as well as "
        "Identify with CNS values for 1.2spec tests");
}

DeleteAllNamespacesAndVerify::~DeleteAllNamespacesAndVerify()
{
    ///////////////////////////////////////////////////////////////////////////
    // Allocations taken from the heap and not under the control of the
    // RsrcMngr need to be freed/deleted here.
    ///////////////////////////////////////////////////////////////////////////
}

DeleteAllNamespacesAndVerify::
DeleteAllNamespacesAndVerify(const DeleteAllNamespacesAndVerify &other) : Test(other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
}

DeleteAllNamespacesAndVerify &
DeleteAllNamespacesAndVerify::operator=(const DeleteAllNamespacesAndVerify &other)
{
    ///////////////////////////////////////////////////////////////////////////
    // All pointers in this object must be NULL, never allow shallow or deep
    // copies, see Test::Clone() header comment.
    ///////////////////////////////////////////////////////////////////////////
    Test::operator=(other);
    return *this;
}

Test::RunType
DeleteAllNamespacesAndVerify::RunnableCoreTest(bool preserve)
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
DeleteAllNamespacesAndVerify::RunCoreTest()
{
    /** \verbatim
     * Assumptions:
     * None.
     * \endverbatim
     */
	LOG_NRM("Start DeleteAllNamespacesAndVerify::RunCoreTest");

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
    const uint8_t *pROPrpBuffer = NULL;
    const uint32_t *namespaceIdListBuffer32BitPtr = NULL;
    const uint16_t *controllerIdListBuffer16BitPtr;

    ConstSharedIdentifyPtr idCtrlrCap = gInformative->GetIdentifyCmdCtrlr();
    uint32_t identifyControllerMaxNSID = (uint32_t) idCtrlrCap->GetValue(IDCTRLRCAP_NN);
    uint32_t currentControllerIdToDelete = 0;
    uint32_t currentNamespaceIdToDelete = 0;
    uint32_t numDeletedNamespaces = 0;
    uint32_t numDetachedNamespaces = 0;
    //CEStat returnStat;

	LOG_NRM("Create Identify Command To Get All Present NSIDs and buffers to hold a namespace struct, namespace list, and controller list.");
	SharedIdentifyPtr identifyCmd = SharedIdentifyPtr(new Identify());
    identifyCmd->SetCNTID(0); // Current CNTID is implied in this command
    SharedMemBufferPtr identifyNamespaceStruct = SharedMemBufferPtr(new MemBuffer());
    identifyNamespaceStruct->Init(4096, true, 0x0);
    SharedMemBufferPtr identifyNamespaceList = SharedMemBufferPtr(new MemBuffer());
    identifyNamespaceList->Init(4096, true, 0x0);
    SharedMemBufferPtr identifyControllerList = SharedMemBufferPtr(new MemBuffer());
    identifyControllerList->Init(4096, true, 0x0);

	LOG_NRM("Create NamespaceAttach Command to detach all namespaces that are presently attached to this controller, and buffer.");
    SharedNamespaceAttachPtr namespaceAttachCmd = SharedNamespaceAttachPtr(new NamespaceAttach() );
    SharedMemBufferPtr attachBuffer = SharedMemBufferPtr(new MemBuffer() );
    attachBuffer->Init(4096, true, 0x0);

	LOG_NRM("Create NamespaceManagement to delete all namespaces after detaching from this controller, and buffer.");
    SharedNamespaceManagementPtr namespaceManagementCmd = SharedNamespaceManagementPtr(new NamespaceManagement() );
    SharedMemBufferPtr managementBuffer = SharedMemBufferPtr(new MemBuffer() );
    managementBuffer->Init(384, true, 0x0); // Would contain the namespace struct info for namespace management to consume (384B)
    namespaceManagementCmd->SetPrpBuffer(prpBitmask, managementBuffer); // Only required for creating namespaces...

    // Figure 86 in 1.2 spec               CNS  NSID CNTID Returns
    // CNS_Namespace                      0x00    X     -   (Identify Namespace from NSID X. If attached, receive struct, else all 0's, else invalid namespace ID)
    // CNS_Controller                     0x01    -     -   (Identify Controller struct)
    // CNS_NamespaceListAttached          0x02    Y     -   (Identify Namespace LIST, starting at NSID Y and in an increasing order)
    // CNS_NamespaceListSubsystem         0x10    Z     -   (Identify Namespace LIST, starting at NSID Z present in subsystem that may or may not be attached)
    // CNS_NamespaceStructSubsystem       0x11    X     -   (Identify Namespace from NSID A. If attached or not, receive the struct, else invalid namespace ID)
    // CNS_ControllerListAttachedToNSID   0x12    X     A   (Controller List that are attached to NSID X, starting with CNTID greater than A)
    // CNS_ControllerListSubsystem        0x13    -     B   (Controller List present in subsystem starting with CNTID greater than B)

	LOG_NRM("Start main loop pulling back NS lists to detach/delete each NSID");
	// See 8.11 in 1.2 spec for describing the process this test is following
	bool namespacesRemaining = true;
    while(namespacesRemaining == true) {

    	LOG_NRM("Setup Identify Command to retrieve a NSID list. Could be larger than 1024, therefore looping. Current potential list starts at NSID > 0x%llx", (long long unsigned) currentNamespaceIdToDelete);
    	identifyCmd->SetCNS(CNS_NamespaceListSubsystem); // Get all NSIDs attached or not to this controller
    	identifyCmd->SetNSID(currentNamespaceIdToDelete); // This should be 0/1024/2048/etc
    	identifyCmd->SetPrpBuffer(prpBitmask, identifyNamespaceList);
        IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq, identifyCmd, "Read back (up to) 1024 NSIDs attached to this controller (CNS=2)", false, CESTAT_SUCCESS);
        namespaceIdListBuffer32BitPtr = (uint32_t*) identifyCmd->GetROPrpBuffer();

        LOG_NRM("NSID list has been read, loop over the buffer contents, a value of 0 marks the end of the list, NSIDs are required to increment otherwise");
        for(uint32_t currentNamespaceIndex = 0; currentNamespaceIndex < 1024; currentNamespaceIndex++) {

        	if(currentNamespaceIdToDelete > namespaceIdListBuffer32BitPtr[currentNamespaceIndex] && namespaceIdListBuffer32BitPtr[currentNamespaceIndex] != 0) {
    			throw FrmwkEx(HERE, "Found an NSID in the namespace list that is non-zero and was not larger than the previous NSID. The list is required to acend in order.");
        	}

        	currentNamespaceIdToDelete = namespaceIdListBuffer32BitPtr[currentNamespaceIndex];
        	if (currentNamespaceIdToDelete == 0) {
        		LOG_NRM("At index %d of NSID List, found a zero entry. Total namespaces dettached is %d and deleted has been %d", currentNamespaceIndex, (uint32_t) numDetachedNamespaces, (uint32_t) numDeletedNamespaces);
        		namespacesRemaining = false;
				break;
        	}

        	if (currentNamespaceIdToDelete > identifyControllerMaxNSID) {
    			LOG_NRM("Found NSID value in returned NS list at buffer index 0x%08x  value 0x%x that is greater than IdCtrlr.NN's 0x%x", currentNamespaceIndex, (uint32_t) currentNamespaceIdToDelete, (uint32_t) identifyControllerMaxNSID );
    			throw FrmwkEx(HERE, "NSID found in NS List that is greater than CNTLR.NN. Violates 1.6.18 of NVMe 1.2 spec");
        	}

        	// For each NSID in the returned list, detach from all controllers stated it is attached to, then delete
        	// Get the controller list that are stated to be attached to this NSID
        	LOG_NRM("NSID 0x%llx. From namespace list, will be dettached and deleted.", (long long unsigned) currentNamespaceIdToDelete);
        	bool controllersRemaining = true;
        	currentControllerIdToDelete = 0;
        	while( controllersRemaining == true) {
            	LOG_NRM("NSID 0x%llx. Check to see if any contollers are attached", (long long unsigned) currentNamespaceIdToDelete);

				identifyCmd->SetCNS( CNS_ControllerListAttachedToNSID ); // Get all CTRLIDs that are attached to this NSID
				identifyCmd->SetCNTID( currentControllerIdToDelete ); // This number increases per iteration if more than 2047 read the previous time
				identifyCmd->SetNSID( currentNamespaceIdToDelete ); // This should be 0/1024/2048/etc
				identifyCmd->SetPrpBuffer(prpBitmask, identifyControllerList);
				IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq, identifyCmd, "Return up to 2047 controllers attached to this NSID", false, CESTAT_SUCCESS);
				controllerIdListBuffer16BitPtr = (uint16_t*) identifyCmd->GetROPrpBuffer();
				LOG_NRM("NSID %d was found to be attached to at least %d controllers", currentNamespaceIdToDelete, controllerIdListBuffer16BitPtr[0]);

				//BUGBUG The APL firmware isn't reporting back the right controller ID list.. make one with IDs 0/1
				/*
				LOG_NRM("BUGBUG For APL FW, change the controller list to CTRLIDs 0 and 1, instead of 1 and 2")
				if( controllerIdListBuffer16BitPtr[0] > 0) {
					for(uint32_t bufferIndex = 0; bufferIndex < 4096; bufferIndex++) {
						userBuffer[bufferIndex] = 0;
					}
					userBuffer[0] = 2;
					userBuffer[4] = 1; // Set controller IDs to 0/1 only...
					identifyControllerList->InitAlignment(4096, 4096, false, 0, userBuffer);
				}
				*/
				// End BUGBUG

				// Detach if controller list has more than one controller present...
		        if(controllerIdListBuffer16BitPtr[0] > 0) {
					LOG_NRM("Detaching NSID 0x%x from controller list, expecting pass", (uint32_t) currentNamespaceIdToDelete );
					namespaceAttachCmd->SetSEL( CNS_Controller ); // 1 = Detach, 0 = Attach
					namespaceAttachCmd->SetNSID( currentNamespaceIdToDelete );
					namespaceAttachCmd->SetPrpBuffer(prpBitmask, identifyControllerList); // Same list we just pulled for Identify
					IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq, namespaceAttachCmd, "Detaching a namespace from controller list", false, CESTAT_SUCCESS);

					// BUG... If Namespace Attribute Notices are enabled... check for AER here
					if( (idCtrlrCap->GetValue(IDCTRLRCAP_OAES) && 0x00000100) > 0 ) {
						LOG_NRM("Identify Controller's OAES states that NS Attribe Changed events are supported, AER should be submitted to other attached controllers");
					}

					// Loop exit case and counter increase qualifications
					if(controllerIdListBuffer16BitPtr[0] != 2047) {
						controllersRemaining = false;
						LOG_NRM("Last controller list pulled for this NSID contained less than 2047 controller IDs, no more to detach.");
					} else {
						currentControllerIdToDelete += 2047;
						LOG_NRM("Last controller list pulled for this NSID contained 2047 controller IDs, try to pull more");
					}

					// Attempt to detach a namespace that should not have any controllers attached to it.
					LOG_NRM("Detaching NSID 0x%x from controller again, expecting 'not attached' response", (uint32_t) currentNamespaceIdToDelete);
					IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq, namespaceAttachCmd, "Detaching a namespace that was already detached", false, CESTAT_NS_NOT_ATTACHED);

		        } else {
		        	LOG_NRM("Identify Namespace CNS 0x12 returned an emtpy controller list for this NSID. No controller attached, but NSID was not yet deleted. This is odd but ok.");
					controllersRemaining = false;
		        	break;
		        } // End If Controllers Attached to current NSID
        	} // End While detaching controllers

        	// This NSID is now inactive, as it is detached from all controllers in subsystem... but not invalid. Should return all 0's
        	LOG_NRM("Issuing Identify Namespace (CNS=0) to this fully detached (inactive) NSID and verifying that the buffer is all 0's.");
        	identifyCmd->SetCNS(  CNS_Namespace );
        	identifyCmd->SetPrpBuffer(prpBitmask, identifyNamespaceStruct);
        	// Potentially read back 1024 namespaces attacjed to this controller ID
            IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq, identifyCmd, "Reading back NSID that was just detached, should return all 0's", false, CESTAT_SUCCESS);
        	for(uint32_t bufferIndex = 0; bufferIndex < 4096; bufferIndex++) {
        		if (identifyCmd->GetROPrpBuffer()[bufferIndex] != 0) {
        			throw FrmwkEx(HERE, "Expected Identify(CNS=0, NSID=0x%x, CNTID=currentController) returned a non-zero identify buffer after detaching, expecting all zeros.");
        		}
        	}

        	// BUGBUG This Fails On APL Alpha FW
        	LOG_NRM("Issuing Identify Namespace (CNS=0x11) to this inactive NSID and expeting STATUS 0:2 (Invalid Field) to be returned.");
        	identifyCmd->SetCNS(  0x11 );
        	// Potentially read back 1024 namespaces attacjed to this controller ID
            IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq, identifyCmd, "Reading back NSID that was just detached CNS=0x12, should return Invalid Namespace", false, CESTAT_INVAL_FIELD);


        	LOG_NRM("Deleting  NSID 0x%x, expecting pass", (uint32_t) currentNamespaceIdToDelete);
        	namespaceManagementCmd->SetSEL( 0x1 ); // 1 = Delete, 0 = Create
        	// No buffer is required for deletion / SEL = 1. Was cleared to 0's prior to this loop
        	IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq, namespaceManagementCmd, "Deleting an inactive namespace", false, CESTAT_SUCCESS);

        	LOG_NRM("Deleting  NSID 0x%x (again), expecting 'invalid namespace'", (uint32_t) currentNamespaceIdToDelete);
        	IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq, namespaceManagementCmd, "Deleting (again) an inactive namespace", false, CESTAT_INVAL_FIELD);


        	// BUGBUG This Fails On APL Alpha FW
        	LOG_NRM("Issuing Idnetify Namespace (CNS=0x0) to this deleted NSID (now invalid), after it was detached from all controllers, should return Invalid Namespace.");
        	identifyCmd->SetCNS( CNS_Namespace );
        	IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq, identifyCmd, "After deleting namespace, should receive invalid NSID status when we identify with CNS=0", false, CESTAT_INVAL_FIELD);

        	// BUGBUG This Fails On APL Alpha FW
        	LOG_NRM("Issuing Identify Namespace (CNS=0x11) to this deleted NSID (now invalid) and expecting Invalid Namespace.");
        	identifyCmd->SetCNS(  0x11 );
        	// Potentially read back 1024 namespaces attacjed to this controller ID
            IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq, identifyCmd, "After deleting namespace, should receive invalid NSID status when we identify with CNS=11", false, CESTAT_INVAL_FIELD);


        	numDeletedNamespaces++;
        }
    }

    // At this point there should be zero attached namespaces to any controllers in this subsystem, let's verify that via CNS=0x10
	identifyCmd->SetNSID( 0 ); // Expecting no NSIDs to return that are > 0
	identifyCmd->SetCNS( CNS_NamespaceListSubsystem );
    IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq, identifyCmd, "Sanity checking that all NSIDs have been removed theom the controller (CNS=2)", false, CESTAT_SUCCESS);
    pROPrpBuffer = identifyCmd->GetROPrpBuffer();
	for(uint32_t bufferIndex = 0; bufferIndex < 4096; bufferIndex++) {
		if (pROPrpBuffer[bufferIndex] != 0) {
			LOG_NRM("Found non-zero value in returned NS list at buffer index 0x%08x  value  0x%02x", bufferIndex, pROPrpBuffer[bufferIndex]);
			throw FrmwkEx(HERE, "Expected Identify(CNS=0x10, NSID=0, CNTID=0) to not return any NSIDs after detaching and deleting");
		}
	}

    LOG_NRM("Completed DeleteAllNamespacesAndVerify::RunCoreTest");
}

}   // namespace
