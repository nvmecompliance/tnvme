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

    mIdentifyCmdCap = Identify::NullIdentifyPtr;
    mIdentifyCmdNamspc.clear();
    mGetFeaturesNumOfQ = 0;
}


void
Informative::SetIdentifyCmdCapabilities(SharedIdentifyPtr idCmdCap)
{
    // out with the old and in with the new
    mIdentifyCmdCap = Identify::NullIdentifyPtr;
    mIdentifyCmdCap = idCmdCap;
}


    vector<SharedIdentifyPtr> mIdentifyCmdNamspc;
void
Informative::SetIdentifyCmdNamespace(SharedIdentifyPtr idCmdNamspc)
{
    // out with the old and in with the new
    mIdentifyCmdNamspc.clear();
    mIdentifyCmdNamspc.push_back(idCmdNamspc);
}


    uint32_t mGetFeaturesNumOfQ;
void
Informative::SetGetFeaturesNumberOfQueues(uint32_t ceDword0)
{
    // out with the old and in with the new
    mGetFeaturesNumOfQ = 0;
    mGetFeaturesNumOfQ =  ceDword0;
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


vector<uint32_t>
Informative::GetBareNamespaces()
{
    LBAFormat lbaFmt;
    vector<uint32_t> ns;
    ConstSharedIdentifyPtr nsPtr;

    // Determine the Number of Namespaces (NN)
    ConstSharedIdentifyPtr idCmdCap = GetIdentifyCmdCapabilities();
    uint32_t nn = (uint32_t)idCmdCap->GetValue(IDCTRLRCAP_NN);

    // Bare namespaces supporting no meta data, and E2E is disabled;
    // Implies: Identify.LBAF[Identify.FLBAS].MS=0
    for (uint64_t i = 1; i <= nn; i++) {
        nsPtr =  GetIdentifyCmdNamespace(i);
        lbaFmt = nsPtr->GetLBAFormat();
        if (lbaFmt.MS == 0)
            ns.push_back(i);
    }
    return ns;
}


vector<uint32_t>
Informative::GetMetaNamespaces()
{
    uint8_t dps;
    LBAFormat lbaFmt;
    vector<uint32_t> ns;
    ConstSharedIdentifyPtr nsPtr;

    // Determine the Number of Namespaces (NN)
    ConstSharedIdentifyPtr idCmdCap = GetIdentifyCmdCapabilities();
    uint32_t nn = (uint32_t)idCmdCap->GetValue(IDCTRLRCAP_NN);

    // Meta namespaces supporting meta data, and E2E is disabled;
    // Implies: Identify.LBAF[Identify.FLBAS].MS=!0, Identify.DPS_b2:0=0
    for (uint64_t i = 1; i <= nn; i++) {
        nsPtr =  GetIdentifyCmdNamespace(i);
        lbaFmt = nsPtr->GetLBAFormat();
        dps = (uint8_t)nsPtr->GetValue(IDNAMESPC_DPS);
        if ((lbaFmt.MS != 0) && ((dps & 0x07) == 0))
            ns.push_back(i);
    }
    return ns;
}


vector<uint32_t>
Informative::GetE2ENamespaces()
{
    uint8_t dps;
    LBAFormat lbaFmt;
    vector<uint32_t> ns;
    ConstSharedIdentifyPtr nsPtr;

    // Determine the Number of Namespaces (NN)
    ConstSharedIdentifyPtr idCmdCap = GetIdentifyCmdCapabilities();
    uint32_t nn = (uint32_t)idCmdCap->GetValue(IDCTRLRCAP_NN);

    // Meta namespaces supporting meta data, and E2E is disabled;
    // Implies: Identify.LBAF[Identify.FLBAS].MS=!0, Identify.DPS_b2:0=0
    for (uint64_t i = 1; i <= nn; i++) {
        nsPtr =  GetIdentifyCmdNamespace(i);
        lbaFmt = nsPtr->GetLBAFormat();
        dps = (uint8_t)nsPtr->GetValue(IDNAMESPC_DPS);
        if ((lbaFmt.MS != 0) && ((dps & 0x07) != 0))
            ns.push_back(i);
    }
    return ns;
}

