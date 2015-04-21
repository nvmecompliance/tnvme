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

#ifndef _NAMESPACEATTACH_H_
#define _NAMESPACEATTACH_H_

#include "cmd.h"

class NamespaceAttach;    // forward definition
typedef boost::shared_ptr<NamespaceAttach>             SharedNamespaceAttachPtr;
typedef boost::shared_ptr<const NamespaceAttach>       ConstSharedNamespaceAttachPtr;
#define CAST_TO_NAMESPACEATTACH(shared_trackable_ptr)  \
        boost::shared_polymorphic_downcast<NamespaceAttach>(shared_trackable_ptr);


/**
* This class implements the dataset mgmt nvm cmd
*
* @note This class may throw exceptions.
*/
class NamespaceAttach : public Cmd
{
public:
    NamespaceAttach();
    virtual ~NamespaceAttach();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedNamespaceAttachPtr NullNamespaceAttachPtr;
    static const uint8_t Opcode;

    /**
     * @param SEL Pass the value
     */
    void    SetSEL(uint8_t);
    uint8_t GetSEL() const;

};


#endif
