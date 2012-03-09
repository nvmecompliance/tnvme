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

#ifndef _NVMCMD_H_
#define _NVMCMD_H_

#include "cmd.h"


/**
* This class is the base class to NVM cmd set.
*
* @note This class may throw exceptions.
*/
class NVMCmd : public Cmd
{
public:
    /**
     * @param objBeingCreated Pass the type of object this child class is
     */
    NVMCmd(Trackable::ObjType objBeingCreated);
    virtual ~NVMCmd();


protected:
    /**
     * Initialize this object.
     * @param opcode Pass the opcode defining this cmd, per NVME spec.
     * @param dataDir Pass the direction of data for this cmd. This is used
     *      to notify dnvme which way to send base classes PrpData. The kernel
     *      requires special calls dependant upon the direction of xfer. If this
     *      is not correct, unknown outcomes will be observed.
     */
    void Init(uint8_t opcode, DataDir dataDir);
};


#endif
