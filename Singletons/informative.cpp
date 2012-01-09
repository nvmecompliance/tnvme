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

#include "informative.h"
#include "globals.h"


bool Informative::mInstanceFlag = false;
Informative* Informative::mSingleton = NULL;
Informative* Informative::GetInstance(int fd, SpecRev specRev)
{
    if(mInstanceFlag == false) {
        mSingleton = new Informative(fd, specRev);
        mInstanceFlag = true;
        return mSingleton;
    } else {
        return mSingleton;
    }
}
void Informative::KillInstance()
{
    if(mInstanceFlag) {
        mInstanceFlag = false;
        delete mSingleton;
        mSingleton = NULL;
    }
}


Informative::Informative(int fd, SpecRev specRev)
{
    mFd = fd;
    if (mFd < 0) {
        LOG_DBG("Object created with a bad FD=%d", fd);
        return;
    }

    mSpecRev = specRev;

    mIdentifyCmdCap = Identify::NullIdentifyPtr;
    mIdentifyCmdNamspc.clear();
    mGetFeaturesNumOfQ = 0;
}


Informative::~Informative()
{
    mInstanceFlag = false;
}


ConstSharedIdentifyPtr
Informative::GetIdentifyCmdNamespace(uint64_t namspcId)
{
    if (namspcId > mIdentifyCmdNamspc.size()) {
        LOG_DBG("Requested Identify namespace struct %llu, out of %lu",
            (long long unsigned int)namspcId, mIdentifyCmdNamspc.size());
        return Identify::NullIdentifyPtr;
    } else if (namspcId == 0) {
        LOG_DBG("Namespace ID 0 is illegal, must start at 1");
        return Identify::NullIdentifyPtr;
    }

    return mIdentifyCmdNamspc[namspcId-1];
}


uint32_t
Informative::GetFeaturesNumOfQueues()
{
    // Call these 2 methods for logging purposes only
    GetFeaturesNumOfIOCQs();
    GetFeaturesNumOfIOSQs();

    return mGetFeaturesNumOfQ;
}


uint16_t
Informative::GetFeaturesNumOfIOCQs()
{
    LOG_NRM("Max # of IOCQs alloc'd by DUT = %d", (mGetFeaturesNumOfQ >> 16));
    return (uint16_t)(mGetFeaturesNumOfQ >> 16);
}


uint16_t
Informative::GetFeaturesNumOfIOSQs()
{
    LOG_NRM("Max # of IOSQs alloc'd by DUT = %d", (mGetFeaturesNumOfQ & 0xff));
    return (uint16_t)(mGetFeaturesNumOfQ & 0xff);
}
