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
#include "../Exception/frmwkEx.h"


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
    if (mFd < 0)
        throw FrmwkEx(HERE, "Object created with a bad FD=%d", fd);

    mSpecRev = specRev;
    Clear();
}


Informative::~Informative()
{
    mInstanceFlag = false;
    Clear();
}


void
Informative::Clear()
{
    mIdentifyCmdCtrlr = Identify::NullIdentifyPtr;
    mIdentifyCmdNamspc.clear();
    mGetFeaturesNumOfQ = 0;
}


ConstSharedIdentifyPtr
Informative::GetIdentifyCmdCtrlr() const
{
    if (mIdentifyCmdCtrlr == Identify::NullIdentifyPtr)
        throw FrmwkEx(HERE, "Identify data not allowed to be NULL");

    return mIdentifyCmdCtrlr;
}


ConstSharedIdentifyPtr
Informative::GetIdentifyCmdNamspc(uint64_t namspcId) const
{
    if (namspcId > mIdentifyCmdNamspc.size()) {
        LOG_ERR("Requested Identify namspc struct %llu, out of %lu",
            (long long unsigned int)namspcId, mIdentifyCmdNamspc.size());
        return Identify::NullIdentifyPtr;
    } else if (namspcId == 0) {
        LOG_ERR("Namespace ID 0 is illegal, must start at 1");
        return Identify::NullIdentifyPtr;
    }

    if (mIdentifyCmdNamspc[namspcId-1] == Identify::NullIdentifyPtr)
        throw FrmwkEx(HERE, "Identify data not allowed to be NULL");

    return mIdentifyCmdNamspc[namspcId-1];
}


uint32_t
Informative::GetFeaturesNumOfQueues() const
{
    // Call these 2 methods for logging purposes only
    GetFeaturesNumOfIOCQs();
    GetFeaturesNumOfIOSQs();

    return mGetFeaturesNumOfQ;
}


uint32_t
Informative::GetFeaturesNumOfIOCQs() const
{
    uint32_t work = ((mGetFeaturesNumOfQ >> 16) + (uint32_t)1);
    LOG_NRM("Max # of IOCQs alloc'd by DUT = %d",work);

    // This warning is to make people aware that although the Set Features cmd
    // with ID=0x07 is based upon a 0-based number and theoretically states a
    // device could support 0x10000 IOQ's, that in fact a device can never
    // create that many. The Create IOQ cmd's only allow 16 bits to
    // represent a QID and those are 1-based values. Thus QID 0x10000 is not
    // possible and therefore 0x10000 IOQ's are not possible since IOQ's
    // are defined to be those from 1 to 0xffff.
    if (work > 0xffff)
        LOG_WARN("Remember devices cannot support > 0xffff IOQ's");
    return work;
}


uint32_t
Informative::GetFeaturesNumOfIOSQs() const
{
    uint32_t work = ((mGetFeaturesNumOfQ & 0xffff) + (uint32_t)1);
    LOG_NRM("Max # of IOSQs alloc'd by DUT = %d", work);

    // This warning is to make people aware that although the Set Features cmd
    // with ID=0x07 is based upon a 0-based number and theoretically states a
    // device could support 0x10000 IOQ's, that in fact a device can never
    // create that many. The Create IOQ cmd's only allow 16 bits to
    // represent a QID and those are 1-based values. Thus QID 0x10000 is not
    // possible and therefore 0x10000 IOQ's are not possible since IOQ's
    // are defined to be those from 1 to 0xffff.
    if (work > 0xffff)
        LOG_WARN("Remember devices cannot support > 0xffff IOQ's");
    return work;
}


vector<uint32_t>
Informative::GetBareNamespaces() const
{
    vector<uint32_t> ns;
    ConstSharedIdentifyPtr nsPtr;

    // Determine the Number of Namespaces (NN)
    ConstSharedIdentifyPtr idCmdCtrlr = GetIdentifyCmdCtrlr();
    uint32_t nn = (uint32_t)idCmdCtrlr->GetValue(IDCTRLRCAP_NN);

    // Bare namespaces supporting no meta data, and E2E is disabled;
    // Implies: Identify.LBAF[Identify.FLBAS].MS=0
    LOG_NRM("Seeking all bare namspc's");
    for (uint64_t i = 1; i <= nn; i++) {
        nsPtr =  GetIdentifyCmdNamspc(i);
        if (IdentifyNamespace(nsPtr) == NS_BARE) {
            LOG_NRM("Identified bare namspc #%ld", i);
            ns.push_back(i);
        }
    }
    return ns;
}


vector<uint32_t>
Informative::GetMetaNamespaces() const
{
    NamspcType nsType;
    vector<uint32_t> ns;
    ConstSharedIdentifyPtr nsPtr;

    // Determine the Number of Namespaces (NN)
    ConstSharedIdentifyPtr idCmdCtrlr = GetIdentifyCmdCtrlr();
    uint32_t nn = (uint32_t)idCmdCtrlr->GetValue(IDCTRLRCAP_NN);

    LOG_NRM("Seeking all meta namspc's");
    for (uint64_t i = 1; i <= nn; i++) {
        nsPtr =  GetIdentifyCmdNamspc(i);
        nsType = IdentifyNamespace(nsPtr);
        if ((nsType == NS_METAI) || (nsType == NS_METAS)) {
            LOG_NRM("Identified meta namspc with %s buffer (ID = %ld)",
                (nsType == NS_METAI) ? "interleaved" : "separate", i);
            ns.push_back(i);
        }
    }
    return ns;
}


vector<uint32_t>
Informative::GetMetaINamespaces() const
{
    NamspcType nsType;
    vector<uint32_t> ns;
    ConstSharedIdentifyPtr nsPtr;

    // Determine the Number of Namespaces (NN)
    ConstSharedIdentifyPtr idCmdCtrlr = GetIdentifyCmdCtrlr();
    uint32_t nn = (uint32_t)idCmdCtrlr->GetValue(IDCTRLRCAP_NN);

    LOG_NRM("Seeking all interleaved meta namspc's");
    for (uint64_t i = 1; i <= nn; i++) {
        nsPtr =  GetIdentifyCmdNamspc(i);
        nsType = IdentifyNamespace(nsPtr);
        if (nsType == NS_METAI) {
            LOG_NRM(
                "Identified meta namspc with interleaved buffer (ID = %ld)", i);
            ns.push_back(i);
        }
    }
    return ns;
}


vector<uint32_t>
Informative::GetMetaSNamespaces() const
{
    NamspcType nsType;
    vector<uint32_t> ns;
    ConstSharedIdentifyPtr nsPtr;

    // Determine the Number of Namespaces (NN)
    ConstSharedIdentifyPtr idCmdCtrlr = GetIdentifyCmdCtrlr();
    uint32_t nn = (uint32_t)idCmdCtrlr->GetValue(IDCTRLRCAP_NN);

    LOG_NRM("Seeking all separate meta namspc's");
    for (uint64_t i = 1; i <= nn; i++) {
        nsPtr =  GetIdentifyCmdNamspc(i);
        nsType = IdentifyNamespace(nsPtr);
        if (nsType == NS_METAS) {
            LOG_NRM(
                "Identified meta namspc with separate buffer (ID = %ld)", i);
            ns.push_back(i);
        }
    }
    return ns;
}


vector<uint32_t>
Informative::GetE2eNamespaces() const
{
    NamspcType nsType;
    vector<uint32_t> ns;
    ConstSharedIdentifyPtr nsPtr;

    // Determine the Number of Namespaces (NN)
    ConstSharedIdentifyPtr idCmdCtrlr = GetIdentifyCmdCtrlr();
    uint32_t nn = (uint32_t)idCmdCtrlr->GetValue(IDCTRLRCAP_NN);

    LOG_NRM("Seeking all E2E namspc's");
    for (uint64_t i = 1; i <= nn; i++) {
        nsPtr =  GetIdentifyCmdNamspc(i);
        nsType = IdentifyNamespace(nsPtr);
        if ((nsType == NS_E2EI) || (nsType == NS_E2ES)) {
            LOG_NRM("Identified E2E namspc with %s buffer (ID = %ld)",
                (nsType == NS_E2EI) ? "interleaved" : "separate", i);
            ns.push_back(i);
        }
    }
    return ns;
}


vector<uint32_t>
Informative::GetE2eINamespaces() const
{
    NamspcType nsType;
    vector<uint32_t> ns;
    ConstSharedIdentifyPtr nsPtr;

    // Determine the Number of Namespaces (NN)
    ConstSharedIdentifyPtr idCmdCtrlr = GetIdentifyCmdCtrlr();
    uint32_t nn = (uint32_t)idCmdCtrlr->GetValue(IDCTRLRCAP_NN);

    LOG_NRM("Seeking all interleaved E2E namspc's");
    for (uint64_t i = 1; i <= nn; i++) {
        nsPtr =  GetIdentifyCmdNamspc(i);
        nsType = IdentifyNamespace(nsPtr);
        if (nsType == NS_E2EI) {
            LOG_NRM(
                "Identified E2E namspc with interleaved buffer (ID = %ld)", i);
            ns.push_back(i);
        }
    }
    return ns;
}


vector<uint32_t>
Informative::GetE2eSNamespaces() const
{
    NamspcType nsType;
    vector<uint32_t> ns;
    ConstSharedIdentifyPtr nsPtr;

    // Determine the Number of Namespaces (NN)
    ConstSharedIdentifyPtr idCmdCtrlr = GetIdentifyCmdCtrlr();
    uint32_t nn = (uint32_t)idCmdCtrlr->GetValue(IDCTRLRCAP_NN);

    LOG_NRM("Seeking all separate E2E namspc's");
    for (uint64_t i = 1; i <= nn; i++) {
        nsPtr =  GetIdentifyCmdNamspc(i);
        nsType = IdentifyNamespace(nsPtr);
        if (nsType == NS_E2ES) {
            LOG_NRM("Identified E2E namspc with separate buffer (ID = %ld)", i);
            ns.push_back(i);
        }
    }
    return ns;
}


Informative::Namspc
Informative::Get1stBareMetaE2E() const
{
    vector<uint32_t> namspc;

    namspc= GetBareNamespaces();
    if (namspc.size())
        return (Namspc(GetIdentifyCmdNamspc(namspc[0]), namspc[0], NS_BARE));

    namspc = GetMetaSNamespaces();
    if (namspc.size())
        return (Namspc(GetIdentifyCmdNamspc(namspc[0]), namspc[0], NS_METAS));

    namspc = GetMetaINamespaces();
    if (namspc.size())
        return (Namspc(GetIdentifyCmdNamspc(namspc[0]), namspc[0], NS_METAI));

    namspc = GetE2eSNamespaces();
    if (namspc.size())
        return (Namspc(GetIdentifyCmdNamspc(namspc[0]), namspc[0], NS_E2ES));

    namspc = GetE2eINamespaces();
    if (namspc.size())
        return (Namspc(GetIdentifyCmdNamspc(namspc[0]), namspc[0], NS_E2EI));

    throw FrmwkEx(HERE, "DUT must have 1 of 3 namspc's");
}


Informative::NamspcType
Informative::IdentifyNamespace(ConstSharedIdentifyPtr idCmdNamspc) const
{
    LBAFormat lbaFmt;
    uint8_t dps;
    uint8_t flbas;

    // Bare namespaces: Namspc's supporting no meta data, and E2E is disabled;
    // Implies: Identify.LBAF[Identify.FLBAS].MS=0
    lbaFmt = idCmdNamspc->GetLBAFormat();
    if (lbaFmt.MS == 0) {
        return NS_BARE;
    }

    // Learn more about this namespace to decipher its classification/type
    dps = (uint8_t)idCmdNamspc->GetValue(IDNAMESPC_DPS);
    flbas = (uint8_t)idCmdNamspc->GetValue(IDNAMESPC_DPS);

    if ((dps & 0x07) == 0) {
        // Meta namespaces supporting meta data, and E2E is disabled;
        // Implies: Identify.LBAF[Identify.FLBAS].MS=!0, Identify.DPS_b2:0=0
        if (flbas & (1 << 4))
            return NS_METAI;
        else
            return NS_METAS;
    } else {
        // Meta namespaces supporting meta data, and E2E is disabled;
        // Implies: Identify.LBAF[Identify.FLBAS].MS=!0, Identify.DPS_b2:0=0
        if (flbas & (1 << 4))
            return NS_E2EI;
        else
            return NS_E2ES;
    }
    throw FrmwkEx(HERE, "Namspc is unidentifiable");
}

