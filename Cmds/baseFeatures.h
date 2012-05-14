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

#ifndef _BASEFEATURES_H_
#define _BASEFEATURES_H_

#include "cmd.h"


/**
* This class implements the common operations of Get/Set BaseFeatures admin cmd
*
* @note This class may throw exceptions.
*/
class BaseFeatures : public Cmd
{
public:
    /**
     * @param objBeingCreated Pass the type of object this child class is
     */
    BaseFeatures(Trackable::ObjType objBeingCreated);
    virtual ~BaseFeatures();

    /**
     * Set which features ID to retrieve from the DUT.
     * @param fid Pass which ID to request
     */
    void SetFID(uint8_t fid);
    uint8_t GetFID() const;
    static const uint8_t FID_ARBITRATION;        // admin cmd
    static const uint8_t FID_PWR_MGMT;           // admin cmd
    static const uint8_t FID_LBA_RANGE;          // admin cmd
    static const uint8_t FID_TEMP_THRESHOLD;     // admin cmd
    static const uint8_t FID_ERR_RECOVERY;       // admin cmd
    static const uint8_t FID_VOL_WR_CACHE;       // admin cmd; volatile wr cache
    static const uint8_t FID_NUM_QUEUES;         // admin cmd
    static const uint8_t FID_IRQ_COALESCING;     // admin cmd
    static const uint8_t FID_IRQ_VEC_CONFIG;     // admin cmd
    static const uint8_t FID_WRITE_ATOMICITY;    // admin cmd
    static const uint8_t FID_ASYNC_EVENT_CONFIG; // admin cmd
    static const uint8_t FID_SW_PROGRESS;        // NVM cmd


private:
    BaseFeatures();
};


#endif
