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
    // For complete details about the APPEND_TEST_AT_?LEVEL() macros:
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering" and
    // "https://github.com/nvmecompliance/tnvme/wiki/Test-Strategy
    switch (gCmdLine.rev) {
    case SPECREV_10b:
    case SPECREV_11:
        break;
    case SPECREV_12:
        APPEND_TEST_AT_XLEVEL(CreateResources_r12, GrpAdminNamespaceManagement)
        APPEND_TEST_AT_YLEVEL(DeleteAllNamespacesAndVerify, GrpAdminNamespaceManagement)
        APPEND_TEST_AT_YLEVEL(CreateAndAttachMaxNamespacesAndVerify, GrpAdminNamespaceManagement)
        //APPEND_TEST_AT_XLEVEL(NamespaceAttachment,  GrpAdminNamespaceManagement)
        break;
    default:
    case SPECREVTYPE_FENCE:
        throw FrmwkEx(HERE, "Object created with an unknown SpecRev=%d",
            gCmdLine.rev);
    }
}

GrpAdminNamespaceManagement::~GrpAdminNamespaceManagement()
{
    // mTests deallocated in parent
}

}   // namespace
