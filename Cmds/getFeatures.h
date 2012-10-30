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

#ifndef _GETFEATURES_H_
#define _GETFEATURES_H_

#include "baseFeatures.h"


class GetFeatures;    // forward definition
typedef boost::shared_ptr<GetFeatures>              SharedGetFeaturesPtr;
typedef boost::shared_ptr<const GetFeatures>        ConstSharedGetFeaturesPtr;
#define CAST_TO_GETFEATURES(shared_trackable_ptr)   \
        boost::shared_polymorphic_downcast<GetFeatures>(shared_trackable_ptr);


/**
* This class implements the GetFeatures admin cmd
*
* @note This class may throw exceptions.
*/
class GetFeatures : public BaseFeatures
{
public:
    GetFeatures();
    virtual ~GetFeatures();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedGetFeaturesPtr NullGetFeaturesPtr;
    static const uint8_t Opcode;

    /**
     * Set the interrupt vector desired for getting the ivec configuration
     * using get features cmd.
     * @param iv Specify the interrupt vec for which the config settings needed.
     */
    void SetIntVecConfigIV(uint16_t iv);
};


#endif
