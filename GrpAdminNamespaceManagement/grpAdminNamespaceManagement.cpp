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

#include "grpAdminNamespaceManagement.h"
#include "createResources_r12.h"
//#include "namespaceAttachment.h"
#include "deleteAllNamespacesAndVerify.h"
#include "createAndAttachMaxNamespacesAndVerify.h"

namespace GrpAdminNamespaceManagement {

GrpAdminNamespaceManagement::GrpAdminNamespaceManagement(size_t grpNum) :
		Group(grpNum, "GrpAdminNamespaceManagement", "Namespace Management")
{
    APPEND_TEST_AT_XLEVEL(CreateResources_r12,           GrpAdminNamespaceManagement)
	APPEND_TEST_AT_XLEVEL(DeleteAllNamespacesAndVerify,  GrpAdminNamespaceManagement)
	APPEND_TEST_AT_XLEVEL(CreateAndAttachMaxNamespacesAndVerify,  GrpAdminNamespaceManagement)
    //APPEND_TEST_AT_XLEVEL(NamespaceAttachment,  GrpAdminNamespaceManagement)
}

GrpAdminNamespaceManagement::~GrpAdminNamespaceManagement()
{
    // mTests deallocated in parent
}

}   // namespace
