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

#ifndef _ADMINCMD_H_
#define _ADMINCMD_H_

#include "cmd.h"


class AdminCmd;    // forward definition
typedef boost::shared_ptr<AdminCmd>             SharedAdminCmdPtr;
typedef boost::shared_ptr<const AdminCmd>       ConstSharedAdminCmdPtr;
#define CAST_TO_ADMINCMD(shared_trackable_ptr)  \
    boost::shared_polymorphic_downcast<AdminCmd>(shared_trackable_ptr);


/**
* This class is the base class to admin cmd set.
*
* @note This class may throw exceptions.
*/
class AdminCmd : public Cmd
{
public:
    /**
     * @param objBeingCreated Pass the type of object this child class is
     */
    AdminCmd(Trackable::ObjType objBeingCreated);
    AdminCmd();
    virtual ~AdminCmd();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedAdminCmdPtr NullAdminCmdPtr;

    /**
     * Is intended to initialize this object as a general cmd. Children derived
     * from this class should NOT be using this method. This general cmd will
     * not be allowed to have a PRP payload due to the amorphous definition.
     * @note This initializer will most likley only be useful to issue general
     *       illegal cmds opcodes to a DUT.
     * @param opcode Pass the opcode defining this cmd.
     * @param cmdSize Pass the number of bytes consisting of a single cmd.
     */
    void Init(uint8_t opcode, uint16_t cmdSize = 64);
};


#endif
