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

#ifndef _BASESECURITY_H_
#define _BASESECURITY_H_

#include "cmd.h"

/**
* This class is the base class to Security Send/Receive cmd classes. It is not
* meant to be instantiated. This class contains all things common to security
* send and receive cmds at a high level. After instantiation by a child the
* Init() method must be called be to attain something useful.
*
* @note This class may throw exceptions.
*/
class BaseSecurity : public Cmd
{
public:
    /**
     * @param objBeingCreated Pass the type of object this child class is
     */
    BaseSecurity(Trackable::ObjType objBeingCreated);
    virtual ~BaseSecurity();

    /**
     * Set the Security Protocol (SECP) field on Dword 10 at 31:24
     * @param sepc Pass the new value of the SEPC field
     */
    void SetSECP(uint8_t secp);
    uint8_t GetSECP() const;

    /**
     * Set the SP Specific (SPSP) field on Dword 10 at 23:08
     * @param spsp Pass the new value of the SPSP field
     */
    void SetSPSP(uint16_t spsp);
    uint16_t GetSPSP() const;

    /**
     * Set the Transfer Length (TL) field on Dword 11 at 31:16
     * @param tl Pass the new value of the TL field
     */
    void SetTL(uint16_t tl);
    uint16_t GetTL() const;


private:
    BaseSecurity();
};


#endif
