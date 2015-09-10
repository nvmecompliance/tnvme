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


private:
    BaseFeatures();
};


#endif
