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
#include "../Cmds/getFeatures.h"
#include "../Utils/kernelAPI.h"
#include "../Utils/io.h"

#define GRP_NAME        "singleton"
#define TEST_NAME       "informative"


bool Informative::mInstanceFlag = false;
Informative *Informative::mSingleton = NULL;
Informative *Informative::GetInstance(int fd, SpecRev specRev)
{
    LOG_NRM("Instantiating global Informative object");
    if(mInstanceFlag == false) {
        mSingleton = new Informative(fd, specRev);
        mInstanceFlag = true;

        if (mSingleton->Init() == false) {
            mSingleton->Clear();
            delete mSingleton;
            mSingleton = NULL;
            mInstanceFlag = false;
        }
    }
    return mSingleton;
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
    flbas = (uint8_t)idCmdNamspc->GetValue(IDNAMESPC_FLBAS);

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


bool
Informative::Init()
{
    bool status = true;

    try {   // The objects to perform this work throw exceptions
        // Clean out any garbage in the dump directories
        FileSystem::SetBaseDumpDir(false);
        if (FileSystem::CleanDumpDir() == false) {
            LOG_ERR("Unable to clean dump dir\n");
            throw FrmwkEx(HERE);
        }
        FileSystem::SetBaseDumpDir(true);
        if (FileSystem::CleanDumpDir() == false) {
            LOG_ERR("Unable to clean dump dir\n");
            throw FrmwkEx(HERE);
        }

        if (gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY) == false)
            throw FrmwkEx(HERE);

        LOG_NRM("Prepare the admin Q's to setup this request");
        SharedACQPtr acq = SharedACQPtr(new ACQ(gDutFd));
        acq->Init(2);
        SharedASQPtr asq = SharedASQPtr(new ASQ(gDutFd));
        asq->Init(2);
        gCtrlrConfig->SetCSS(CtrlrConfig::CSS_NVM_CMDSET);
        if (gCtrlrConfig->SetState(ST_ENABLE) == false)
            throw FrmwkEx(HERE);

        status = Reinit(asq, acq, CALC_TIMEOUT_ms(1));
    } catch (...) {
        LOG_ERR("Failed to init Informative singleton");
        status = false;
    }

    gCtrlrConfig->SetState(ST_DISABLE_COMPLETELY);
    return status;
}


bool
Informative::Reinit(SharedASQPtr &asq, SharedACQPtr &acq, uint16_t ms)
{
    LOG_NRM("------------gInformative(re/init) START------------");

    // Change dump dir to be compatible for info extraction
    FileSystem::SetBaseDumpDir(true);
    if (FileSystem::RotateDumpDir() == false) {
        LOG_ERR("Unable to rotate dump dir\n");
        throw FrmwkEx(HERE);
    }

    Clear();    // Clear out the old, in with the new

    LOG_NRM("----------------start(dump regs)-------------------");
    KernelAPI::DumpPciSpaceRegs(
        FileSystem::PrepDumpFile(GRP_NAME, TEST_NAME, "pci", "regs"), false);
    KernelAPI::DumpCtrlrSpaceRegs(
        FileSystem::PrepDumpFile(GRP_NAME, TEST_NAME, "ctrl", "regs"), false);
    LOG_NRM("-----------------end(dump regs)--------------------");

    SendGetFeaturesNumOfQueues(asq, acq, ms);
    SendIdentifyCtrlrStruct(asq, acq, ms);
    SendIdentifyNamespaceStruct(asq, acq, ms);

    // Change dump dir to be compatible for test execution
    FileSystem::SetBaseDumpDir(false);
    LOG_NRM("------------gInformative(re/init) END------------");
    return true;
}


void
Informative::SendGetFeaturesNumOfQueues(SharedASQPtr asq, SharedACQPtr acq,
    uint16_t ms)
{
    uint32_t numCE;
    uint32_t isrCount;
    uint16_t uniqueId;


    LOG_NRM("----------------start(get features)-----------------");
    LOG_NRM("Create get features");
    SharedGetFeaturesPtr gfNumQ = SharedGetFeaturesPtr(new GetFeatures());
    LOG_NRM("Force get features to request number of queues");
    gfNumQ->SetFID(GetFeatures::FID_NUM_QUEUES);
    gfNumQ->Dump(
        FileSystem::PrepDumpFile(GRP_NAME, TEST_NAME, "GetFeat", "NumOfQueue"),
        "The get features number of queues cmd");


    LOG_NRM("Send the get features cmd to hdw");
    asq->Send(gfNumQ, uniqueId);
    asq->Dump(FileSystem::PrepDumpFile(GRP_NAME, TEST_NAME, "asq",
        "GetFeat.NumOfQueue"),
        "Just B4 ringing SQ0 doorbell, dump entire SQ contents");
    asq->Ring();


    LOG_NRM("Wait for the CE to arrive in ACQ");
    if (acq->ReapInquiryWaitSpecify(ms, 1, numCE, isrCount) == false) {

        acq->Dump(
            FileSystem::PrepDumpFile(GRP_NAME, TEST_NAME, "acq",
            "GetFeat.NumOfQueue"),
            "Unable to see any CE's in CQ0, dump entire CQ contents");
        throw FrmwkEx(HERE, "Unable to see completion of get features cmd");
    } else if (numCE != 1) {
        acq->Dump(
            FileSystem::PrepDumpFile(GRP_NAME, TEST_NAME, "acq",
            "GetFeat.NumOfQueue"),
            "Unable to see any CE's in CQ0, dump entire CQ contents");
        LOG_ERR("The ACQ should only have 1 CE as a result of a cmd");
        throw FrmwkEx(HERE);
    }
    acq->Dump(FileSystem::PrepDumpFile(GRP_NAME, TEST_NAME, "acq",
        "GetFeat.NumOfQueue"),
        "Just B4 reaping CQ0, dump entire CQ contents");

    {
        uint32_t ceRemain;
        uint32_t numReaped;


        LOG_NRM("The CQ's metrics before reaping holds head_ptr needed");
        struct nvme_gen_cq acqMetrics = acq->GetQMetrics();
        KernelAPI::LogCQMetrics(acqMetrics);

        LOG_NRM("Reaping CE from ACQ, requires memory to hold reaped CE");
        SharedMemBufferPtr ceMemCap = SharedMemBufferPtr(new MemBuffer());
        if ((numReaped = acq->Reap(ceRemain, ceMemCap, isrCount, numCE, true))
            != 1) {

            throw FrmwkEx(HERE,
                "Verified there was 1 CE, but reaping produced %d", numReaped);
        }
        LOG_NRM("The reaped CE is...");
        acq->LogCE(acqMetrics.head_ptr);
        acq->DumpCE(acqMetrics.head_ptr, FileSystem::PrepDumpFile
            (GRP_NAME, TEST_NAME, "CE", "GetFeat.NumOfQueue"),
            "The CE of the Get Features cmd; Number of Q's feature ID:");

        union CE ce = acq->PeekCE(acqMetrics.head_ptr);
        ProcessCE::Validate(ce);  // throws upon error

        // This data is static; allows all tests to extract from common point
        mGetFeaturesNumOfQ = ce.t.dw0;
    }
    LOG_NRM("-----------------end(get features)------------------");
}


void
Informative::SendIdentifyCtrlrStruct(SharedASQPtr asq, SharedACQPtr acq,
    uint16_t ms)
{
    LOG_NRM("----------------start(ID ctrlr struct)-------------------");
    LOG_NRM("Create 1st identify cmd and assoc some buffer memory");
    SharedIdentifyPtr idCmdCtrlr = SharedIdentifyPtr(new Identify());
    LOG_NRM("Force identify to request ctrlr capabilities struct");
    idCmdCtrlr->SetCNS(true);
    SharedMemBufferPtr idMemCap = SharedMemBufferPtr(new MemBuffer());
    idMemCap->InitAlignment(Identify::IDEAL_DATA_SIZE, PRP_BUFFER_ALIGNMENT,
        true, 0);
    send_64b_bitmask prpReq =
        (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    idCmdCtrlr->SetPrpBuffer(prpReq, idMemCap);

    IO::SendAndReapCmd(GRP_NAME, TEST_NAME, ms, asq, acq,
        idCmdCtrlr, "IdCtrlStruct", true);

    // This data is static; allows all tests to extract from common point
    mIdentifyCmdCtrlr = idCmdCtrlr;
    LOG_NRM("-----------------end(ID ctrlr struct)--------------------");
}


void
Informative::SendIdentifyNamespaceStruct(SharedASQPtr asq, SharedACQPtr acq,
    uint16_t ms)
{
    uint64_t numNamSpc;
    char qualifier[20];


    ConstSharedIdentifyPtr idCmdCtrlr = GetIdentifyCmdCtrlr();
    if ((numNamSpc = idCmdCtrlr->GetValue(IDCTRLRCAP_NN)) == 0)
        throw FrmwkEx(HERE, "Required to support >= 1 namespace");

    LOG_NRM("Gather %lld identify namspc structs from DUT",
        (unsigned long long)numNamSpc);
    for (uint64_t namSpc = 1; namSpc <= numNamSpc; namSpc++) {
        LOG_NRM("-------------start(ID namspc %ld struct)------------", namSpc);
        snprintf(qualifier, sizeof(qualifier), "idCmdNamSpc-%llu",
            (long long unsigned int)namSpc);

        LOG_NRM("Create identify cmd #%llu & assoc some buffer memory",
            (long long unsigned int)namSpc);
        SharedIdentifyPtr idCmdNamSpc = SharedIdentifyPtr(new Identify());
        LOG_NRM("Force identify to request namespace struct #%llu",
            (long long unsigned int)namSpc);
        idCmdNamSpc->SetCNS(false);
        idCmdNamSpc->SetNSID(namSpc);
        SharedMemBufferPtr idMemNamSpc = SharedMemBufferPtr(new MemBuffer());
        idMemNamSpc->InitAlignment(Identify::IDEAL_DATA_SIZE,
            PRP_BUFFER_ALIGNMENT, true, 0);
        send_64b_bitmask idPrpNamSpc =
            (send_64b_bitmask)(MASK_PRP1_PAGE | MASK_PRP2_PAGE);
        idCmdNamSpc->SetPrpBuffer(idPrpNamSpc, idMemNamSpc);

        IO::SendAndReapCmd(GRP_NAME, TEST_NAME, ms, asq, acq,
            idCmdNamSpc, qualifier, true);

        // This data is static; allows all tests to extract from common point
        mIdentifyCmdNamspc.push_back(idCmdNamSpc);
        LOG_NRM("-------------end(ID namspc %ld struct)-------------", namSpc);
    }
}
