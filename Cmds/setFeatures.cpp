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

#include "setFeatures.h"
#include "globals.h"
#include "../Utils/buffers.h"

#define BYTE_BITMASK_AB         0x7
#define BYTE_BITMASK_PS         0x1F
#define BYTE_BITMASK_NUM        0x3F
#define BYTE_BITMASK_WCE        0x1
#define BYTE_BITMASK_CD         0x1
#define BYTE_BITMASK_DN         0x1


SharedSetFeaturesPtr SetFeatures::NullSetFeaturesPtr;
const uint8_t SetFeatures::Opcode = 0x09;


SetFeatures::SetFeatures() : BaseFeatures(Trackable::OBJ_SETFEATURES)
{
    Init(Opcode, DATADIR_TO_DEVICE, 64);

    // No cmd should ever be created which violates these masking possibilities
    send_64b_bitmask allowPrpMask = (send_64b_bitmask)
        (MASK_PRP1_PAGE | MASK_PRP2_PAGE);
    SetPrpAllowed(allowPrpMask);
}


SetFeatures::~SetFeatures()
{
}


void
SetFeatures::SetArbitration(uint8_t hpw, uint8_t mpw, uint8_t lpw, uint8_t ab)
{
    LOG_NRM("Setting abritration");
    SetArbitrationHPW(hpw);
    SetArbitrationMPW(mpw);
    SetArbitrationLPW(lpw);
    SetArbitrationAB(ab);
}


void
SetFeatures::SetArbitrationHPW(uint8_t hpw)
{
    LOG_NRM("Setting HPW = 0x%02X", hpw);
    SetByte(hpw, 11, 3);
}


void
SetFeatures::SetArbitrationMPW(uint8_t mpw)
{
    LOG_NRM("Setting MPW = 0x%02X", mpw);
    SetByte(mpw, 11, 2);
}


void
SetFeatures::SetArbitrationLPW(uint8_t lpw)
{
    LOG_NRM("Setting LPW = 0x%02X", lpw);
    SetByte(lpw, 11, 1);
}


void
SetFeatures::SetArbitrationAB(uint8_t ab)
{
    LOG_NRM("Setting AB = 0x%02X", ab);

    uint8_t work = GetByte(11, 0);
    work &= ~BYTE_BITMASK_AB;
    work |= (ab & BYTE_BITMASK_AB);
    SetByte(ab, 11, 0);
}


uint32_t
SetFeatures::GetArbitration() const
{
    LOG_NRM("Getting cmd arbitration");
    return GetDword(11);
}


uint8_t
SetFeatures::GetArbitrationHPW() const
{
    LOG_NRM("Getting HPW");
    return GetByte(11, 3);
}


uint8_t
SetFeatures::GetArbitrationMPW() const
{
    LOG_NRM("Getting MPW");
    return GetByte(11, 2);
}


uint8_t
SetFeatures::GetArbitrationLPW() const
{
    LOG_NRM("Getting LPW");
    return GetByte(11, 1);
}


uint8_t
SetFeatures::GetArbitrationAB() const
{
    LOG_NRM("Getting AB");
    return (GetByte(11, 0) & BYTE_BITMASK_AB);
}


void
SetFeatures::SetPowerManagementPS(uint8_t ps)
{
    LOG_NRM("Setting power state (PS): 0x%02X", ps);

    uint8_t npss = gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_NPSS);
    if (ps > npss)
        throw FrmwkEx(HERE, "PS %d shall not be > NPSS # %d", ps, npss);

    uint8_t work = GetByte(11, 0);
    work &= ~BYTE_BITMASK_PS;
    work |= (ps & BYTE_BITMASK_PS);
    SetByte(work, 11, 0);
}


uint8_t
SetFeatures::GetPowerManagementPS() const
{
    LOG_NRM("Getting power state (PS)");
    return (GetByte(11, 0) & BYTE_BITMASK_PS);
}


void
SetFeatures::SetLBARangeTypeNUM(uint8_t num)
{
    LOG_NRM("Setting number of LBA range (NUM): 0x%02X", num);

    uint8_t work = GetByte(11, 0);
    work &= ~BYTE_BITMASK_NUM;
    work |= (num & BYTE_BITMASK_NUM);
    SetByte(work, 11, 0);
}


uint8_t
SetFeatures::GetLBARangeTypeNUM() const
{
    LOG_NRM("Getting number of LBA range (NUM)");
    return (GetByte(11, 0) & BYTE_BITMASK_NUM);
}


void
SetFeatures::SetTempThresholdTMPTH(uint16_t tmpth)
{
    LOG_NRM("Setting temperature threshold (TMPTH): 0x%04X in Kelvin", tmpth);
    SetWord(tmpth, 11, 0);
}


uint16_t
SetFeatures::GetTempThresholdTMPTH() const
{
    LOG_NRM("Getting temperature threshold (TMPTH)");
    return GetWord(11, 0);
}


void
SetFeatures::SetErrRecoveryTLER(uint16_t tler)
{
    LOG_NRM("Setting error recovery retry timeout (TLER): 0x%04X", tler);
    SetWord(tler, 11, 0);
}


uint16_t
SetFeatures::GetErrRecoveryTLER() const
{
    LOG_NRM("Getting Error recover retry timeout (TLER)");
    return GetWord(11, 0);
}


void
SetFeatures::SetVolatileWriteCacheWCE(uint8_t wce)
{
    LOG_NRM("Setting volatile write cache (WCE): 0x%02X", wce);

    uint8_t work = GetByte(11, 0);
    work &= ~BYTE_BITMASK_WCE;
    work |= (wce & BYTE_BITMASK_WCE);
    SetByte(work, 11, 0);

}


uint8_t
SetFeatures::GetVolatileWriteCacheWCE() const
{
    LOG_NRM("Getting Volatile Write Cache (WCE)");
    return (GetByte(11, 0) & BYTE_BITMASK_WCE);
}


void
SetFeatures::SetNumberOfQueues(uint16_t ncqr, uint16_t nsqr)
{
    LOG_NRM("Setting Set Features(Number of Queues): ncqr=0x%04X, nsqr=0x%04X",
        ncqr, nsqr);

    SetNumberOfQueuesNCQR(ncqr);
    SetNumberOfQueuesNSQR(nsqr);
}


void
SetFeatures::SetNumberOfQueuesNCQR(uint16_t ncqr)
{
    LOG_NRM("Setting Set Features(Number of CQs): NCQR=0x%04X", ncqr);
    SetWord(ncqr, 11, 1);
}


void
SetFeatures::SetNumberOfQueuesNSQR(uint16_t nsqr)
{
    LOG_NRM("Setting Set Features(Number of SQs): NSQR=0x%04X", nsqr);
    SetWord(nsqr, 11, 0);
}


uint32_t
SetFeatures::GetNumberOfQueues() const
{
    LOG_NRM("Getting Set Features(Number of Queues)");
    return GetDword(11);
}


uint16_t
SetFeatures::GetNumberOfQueuesNCQR() const
{
    LOG_NRM("Getting Set Features(Number of CQs): NCQR");
    return GetWord(11, 1);
}


uint16_t
SetFeatures::GetNumberOfQueuesNSQR() const
{
    LOG_NRM("Getting Set Features(Number of SQs): NSQR");
    return GetWord(11, 0);
}


void
SetFeatures::SetIntCoalescing(uint8_t time, uint8_t thr)
{
    LOG_NRM("Setting interrupt coalescing");
    SetIntCoalescingTIME(time);
    SetIntCoalescingTHR(thr);
}


void
SetFeatures::SetIntCoalescingTIME(uint8_t time)
{
    LOG_NRM("Setting aggregation time (TIME): 0x%02X", time);
    SetByte(time, 11, 1);
}


void
SetFeatures::SetIntCoalescingTHR(uint8_t thr)
{
    LOG_NRM("Setting aggregation threshold (THR): 0x%02X", thr);
    SetByte(thr, 11, 0);
}


uint32_t
SetFeatures::GetIntCoalescing() const
{
    LOG_NRM("Getting interrupt coalescing (TIME|THR)");
    return GetDword(11);
}


uint8_t
SetFeatures::GetIntCoalescingTIME() const
{
    LOG_NRM("Getting aggregation time (TIME)");
    return GetByte(11, 1);
}


uint8_t
SetFeatures::GetIntCoalescingTHR() const
{
    LOG_NRM("Getting aggregation threshold (THR)");
    return GetByte(11, 0);
}


void
SetFeatures::SetIntVecConfig(uint8_t cd, uint16_t iv)
{
    LOG_NRM("Setting interrupt vector configuration");

    SetIntVecConfigCD(cd);
    SetIntVecConfigIV(iv);
}


void
SetFeatures::SetIntVecConfigIV(uint16_t iv)
{
    LOG_NRM("Setting interrupt vector (IV): 0x%04X", iv);
    SetWord(iv, 11, 0);
}


void
SetFeatures::SetIntVecConfigCD(uint8_t cd)
{
    LOG_NRM("Setting coalescing disable (CD): 0x%02X", cd);
    uint8_t work = GetByte(11, 2);
    work &= ~BYTE_BITMASK_CD;
    work |= (cd & BYTE_BITMASK_CD);
    SetByte(work, 11, 2);
}


uint32_t
SetFeatures::GetIntVecConfig() const
{
    LOG_NRM("Getting interrupt vector configuration");
    return GetDword(11);
}


uint16_t
SetFeatures::GetIntVecConfigIV() const
{
    LOG_NRM("Getting interrupt vector (IV)");
    return GetWord(11, 0);
}


void
SetFeatures::SetWriteAtomicityDN(uint8_t dn)
{
    LOG_NRM("Setting write atomicity (DN): 0x%02X", dn);
    uint8_t work = GetByte(11, 0);
    work &= ~BYTE_BITMASK_DN;
    work |= (dn & BYTE_BITMASK_DN);
    SetByte(work, 11, 0);
}


uint8_t
SetFeatures::GetWriteAtomicityDN() const
{
    LOG_NRM("Getting Write atomicity (DN)");
    return (GetByte(11, 0)  & BYTE_BITMASK_DN);;
}


uint8_t
SetFeatures::GetIntVecConfigCD() const
{
    LOG_NRM("Getting coalescing disable(CD)");
    return (GetByte(11, 2) & BYTE_BITMASK_CD);
}

void
SetFeatures::SetAsyncEventConfigSMART(uint16_t critWarn)
{
    LOG_NRM("Setting critical warning bits for "
        "(SMART/Health critical warnings): 0x%04X", critWarn);
    SetWord(critWarn, 11, 0);
}


uint16_t
SetFeatures::GetAsyncEventConfigSMART() const
{
    LOG_NRM("Getting critical warning bit vector SMART/Health critical warns)");
    return GetWord(11, 0);
}


void
SetFeatures::SetSWProgressMarkerPBSLC(uint8_t pbslc)
{
    LOG_NRM("Setting software progress marker (PBSLC): 0x%02X", pbslc);
    SetByte(pbslc, 11, 0);
}


uint8_t
SetFeatures::GetSWProgressMarkerPBSLC() const
{
    LOG_NRM("Getting software progress marker (PBSLC)");
    return GetByte(11, 0);
}
