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

#ifndef _NAMESPACEMANAGEMENT_H_
#define _NAMESPACEMANAGEMENT_H_

#include "cmd.h"

class NamespaceManagement;    // forward definition
typedef boost::shared_ptr<NamespaceManagement>             SharedNamespaceManagementPtr;
typedef boost::shared_ptr<const NamespaceManagement>       ConstSharedNamespaceManagementPtr;
#define CAST_TO_NAMESPACEMANAGEMENT(shared_trackable_ptr)  \
        boost::shared_polymorphic_downcast<NamespaceManagement>(shared_trackable_ptr);

/**
* This class implements the dataset mgmt nvm cmd
*
* @note This class may throw exceptions.
*/
class NamespaceManagement : public Cmd
{
public:
    NamespaceManagement();
    virtual ~NamespaceManagement();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedNamespaceManagementPtr NullNamespaceManagementPtr;
    static const uint8_t Opcode;

    /**
     * @param SEL Pass the value
     */
    void    SetSEL(uint8_t);
    uint8_t GetSEL() const;

};


#endif
