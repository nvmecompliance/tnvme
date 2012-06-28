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

#ifndef _FWACTIVATE_H_
#define _FWACTIVATE_H_

#include "cmd.h"


class FWActivate;    // forward definition
typedef boost::shared_ptr<FWActivate>               SharedFWActivatePtr;
typedef boost::shared_ptr<const FWActivate>         ConstSharedFWActivatePtr;
#define CAST_TO_FWACTIVATE(shared_trackable_ptr)    \
        boost::shared_polymorphic_downcast<FWActivate>(shared_trackable_ptr);


/**
* This class implements the FW Activate admin cmd
*
* @note This class may throw exceptions.
*/
class FWActivate : public Cmd
{
public:
    FWActivate();
    virtual ~FWActivate();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedFWActivatePtr NullFWActivatePtr;
    static const uint8_t Opcode;

    /**
     * Set the Activate Action (AA)
     * @param aa Pass any value which can be set in 2 bits.
     */
    void SetAA(uint8_t aa);
    uint8_t GetAA() const;


    /**
     * Set the Firmware Slot (FS)
     * @param fs Pass any value which can be set in 3 bits.
     */
    void SetFS(uint8_t fs);
    uint8_t GetFS() const;
};


#endif
