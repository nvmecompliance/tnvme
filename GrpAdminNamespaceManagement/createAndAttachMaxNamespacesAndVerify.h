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

#ifndef _CREATEANDATTACHMAXNAMESPACESANDVERIFY_r12_H_
#define _CREATEANDATTACHMAXNAMESPACESANDVERIFY_r12_H_

#include "test.h"

namespace GrpAdminNamespaceManagement {

/** \verbatim
 * -----------------------------------------------------------------------------
 * ----------------Mandatory rules for children to follow-----------------------
 * -----------------------------------------------------------------------------
 * 1) See notes in the header file of the Test base class
 * \endverbatim
 */
class CreateAndAttachMaxNamespacesAndVerify : public Test
{
public:
    CreateAndAttachMaxNamespacesAndVerify(string grpName, string testName);
    virtual ~CreateAndAttachMaxNamespacesAndVerify();

    /**
     * IMPORTANT: Read Test::Clone() header comment.
     */
    virtual CreateAndAttachMaxNamespacesAndVerify *Clone() const
        { return new CreateAndAttachMaxNamespacesAndVerify(*this); }
    CreateAndAttachMaxNamespacesAndVerify &operator=(const CreateAndAttachMaxNamespacesAndVerify &other);
    CreateAndAttachMaxNamespacesAndVerify(const CreateAndAttachMaxNamespacesAndVerify &other);
    void CreateNamespaceManagementStructure(uint64_t NSZE, uint64_t NCAP, uint8_t FLBAS, uint8_t DPS, uint8_t NMIC, uint8_t* namespaceManagementBuffer);


protected:
    virtual void RunCoreTest();
    virtual RunType RunnableCoreTest(bool preserve);


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator=().
    ///////////////////////////////////////////////////////////////////////////
};

}   // namespace

#endif
