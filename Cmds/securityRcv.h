/*
 * Copyright (c) 2012, Intel Corporation.
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

#ifndef _SECURITYRCV_H_
#define _SECURITYRCV_H_

#include "baseSecurity.h"


class SecurityRcv;    // forward definition
typedef boost::shared_ptr<SecurityRcv>             SharedSecurityRcvPtr;
typedef boost::shared_ptr<const SecurityRcv>       ConstSharedSecurityRcvPtr;
#define CAST_TO_SECURITYRCV(shared_trackable_ptr)  \
        boost::shared_polymorphic_downcast<SecurityRcv>(shared_trackable_ptr);


/**
* This class implements the Security Send admin cmd specific to nvm cmd set.
*
* @note This class may throw exceptions.
*/
class SecurityRcv : public BaseSecurity
{
public:
    SecurityRcv();
    virtual ~SecurityRcv();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedSecurityRcvPtr NullSecurityRcvPtr;
    static const uint8_t Opcode;
};


#endif
