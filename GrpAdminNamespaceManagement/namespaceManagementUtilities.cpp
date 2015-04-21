/*
 * Copyright (c) 2015, Intel Corporation.
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

/*
	LOG_NRM("Create Identify Command To Get All Present NSIDs");
	SharedIdentifyPtr identifyCmd = SharedIdentifyPtr(new Identify());
	SharedMemBufferPtr identifyNamespaceStruct = SharedMemBufferPtr(new MemBuffer());
	identifyNamespaceStruct->Init(4096, true, 0x0);
	SharedMemBufferPtr identifyNamespaceList = SharedMemBufferPtr(new MemBuffer());
	identifyNamespaceList->Init(4096, true, 0x0);
	prpBitmask = (send_64b_bitmask) (MASK_PRP1_PAGE | MASK_PRP2_PAGE);

	// See 8.11 in 1.2 spec for describing the process this test is following
	identifyCmd->SetNSID( currentNamespaceIdToDelete ); // This should be 0/1024/2048/etc
	// Potentially read back 1024 namespaces attacjed to this controller ID
	IO::SendAndReapCmd(mGrpName, mTestName, CALC_TIMEOUT_ms(1), asq, acq, identifyCmd, "Read back (up to) 1024 NSIDs attached to this controller (CNS=2)", false, CESTAT_SUCCESS, true);
	pROPrpBuffer = identifyCmd->GetROPrpBuffer();
	pROPrpBuffer64B = (long long unsigned int*) pROPrpBuffer;
*/

vector<uint64_t> convertIdentifyNamespaceListBufferToVector(const uint8_t *namespaceListBuffer) {
    const long long unsigned int *namespaceListBuffer64B = (long long unsigned int*) namespaceListBuffer;
    vector<uint64_t> namespaceIDs;

    for(uint32_t namespaceIndex = 0; namespaceIndex < 1024; namespaceIndex++) {
    	if( namespaceListBuffer64B[namespaceIndex] > 0)
    		namespaceIDs.append( namespaceListBUffer64B[namespaceIndex]);
    	break;
    }
    return namespaceIDs;
}

vector<uint32_t> convertIdentifyControllerListBufferToVector(const uint8_t *controllerListBuffer) {
    const long long unsigned int *controllerListBuffer32B = (uint32_t*) controllerListBuffer;
    vector<uint32_t> controllerIDs;
    uint32_t expectedNumControllers = controllerListBuffer32B[0];

    for(uint32_t controllerIndex = 1; controllerIndex < expectedNumControllers + 1; controllerIndex++) {
   		controllerIDs.append( controllerListBUffer32B[controllerIndex]);
    }
    return controllerIDs;
}


