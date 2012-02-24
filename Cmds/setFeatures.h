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

#ifndef _SETFEATURES_H_
#define _SETFEATURES_H_

#include "baseFeatures.h"


class SetFeatures;    // forward definition
typedef boost::shared_ptr<SetFeatures>              SharedSetFeaturesPtr;
typedef boost::shared_ptr<const SetFeatures>        ConstSharedSetFeaturesPtr;
#define CAST_TO_SETFEATURES(shared_trackable_ptr)   \
        boost::shared_polymorphic_downcast<SetFeatures>(shared_trackable_ptr);


/**
* This class implements the SetFeatures admin cmd
*
* @note This class may throw exceptions.
*/
class SetFeatures : public BaseFeatures
{
public:
    SetFeatures(int fd);
    virtual ~SetFeatures();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedSetFeaturesPtr NullSetFeaturesPtr;
    static const uint8_t Opcode;

    /**
     * Set the number of IO queues desired.
     * @note Spec states  "shall only" be set after a power cycle, not resets
     * @param ncqr Pass the number of IOCQ's desired
     * @param nsqr Pass the number of IOSQ's desired
     */
    void SetNumberOfQueues(uint16_t ncqr, uint16_t nsqr);
    uint32_t GetNumberOfQueues() const;


private:
    SetFeatures();
};


#endif
