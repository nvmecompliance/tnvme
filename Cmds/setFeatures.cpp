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
SetFeatures::SetNumberOfQueues(uint16_t ncqr, uint16_t nsqr)
{
    LOG_NRM("Setting Set Features(Number of Queues): ncqr=0x%04X, nsqr=0x%04X",
        ncqr, nsqr);

    uint32_t dw11 = nsqr;
    dw11 |= (((uint32_t)ncqr) << 16);
    SetDword(dw11, 11);
}


uint32_t
SetFeatures::GetNumberOfQueues() const
{
    LOG_NRM("Getting Set Features(Number of Queues)");
    return GetDword(11);
}


void
SetFeatures::SetArbitration(uint32_t arb)
{
    LOG_NRM("Setting cmd arbitation to: 0x%04X", arb);
    SetDword(arb, 11);
}


uint32_t
SetFeatures::GetArbitration() const
{
    LOG_NRM("Getting cmd arbitration");
    return GetDword(11);
}


void
SetFeatures::SetPSD(uint8_t psd)
{
    LOG_NRM("Setting power state descriptor (PSD): 0x%02X", psd);

    uint8_t npss = gInformative->GetIdentifyCmdCtrlr()->
        GetValue(IDCTRLRCAP_NPSS);
    if (psd > 0x1F) {
        throw FrmwkEx(HERE, "Undefined behavior for setting rsvd bits of PSD");
    } else if (psd > npss) {
        throw FrmwkEx(HERE, "PSD # %d shall not be greater than NPSS # %d",
            psd, npss);
    }

    uint32_t dw11 = psd & 0x1F;
    SetDword(dw11, 11);
}


uint8_t
SetFeatures::GetPSD() const
{
    LOG_NRM("Getting power state descriptor (PSD)");
    return GetByte(11, 0);
}


void
SetFeatures::SetTempThreshold(uint16_t tmpth)
{
    LOG_NRM("Setting temperature threshold (TMPTH): 0x%04X in Kelvin", tmpth);

    uint32_t dw11 = tmpth;
    SetDword(dw11, 11);
}


uint16_t
SetFeatures::GetTempThreshold() const
{
    LOG_NRM("Getting temperature threshold (TMPTH)");
    return GetWord(11, 0);
}


void
SetFeatures::SetErrRecoveryTime(uint16_t tler)
{
    LOG_NRM("Setting error recovery retry timeout (TLER): 0x%04X", tler);

    uint32_t dw11 = tler;
    SetDword(dw11, 11);
}


uint16_t
SetFeatures::GetErrRecoveryTime() const
{
    LOG_NRM("Getting Error recover retry timeout (TLER)");
    return GetWord(11, 0);
}


void
SetFeatures::SetVolatileWriteCache(uint8_t wce)
{
    LOG_NRM("Setting volatile write cache (WCE): 0x%02X", wce & 0x1);

    uint32_t dw11 = wce & 0x1;
    SetDword(dw11, 11);
}


uint8_t
SetFeatures::GetVolatileWriteCache() const
{
    LOG_NRM("Getting Volatile Write Cache (WCE)");
    return (GetByte(11, 0) & 0x1);
}

void
SetFeatures::SetIntCoalescing(uint8_t aTime, uint8_t aThr)
{
    LOG_NRM("Setting aggregation time (TIME): 0x%02X", aTime);
    LOG_NRM("Setting aggregation threshold (THR): 0x%02X", aThr);

    uint32_t dw11 = aTime & 0xFF;
    dw11 <<= 8;
    dw11 |= aThr;
    SetDword(dw11, 11);
}


uint16_t
SetFeatures::GetIntCoalescing() const
{
    LOG_NRM("Getting interrupt coalescing (TIME|THR)");
    return GetWord(11, 0);
}


void
SetFeatures::SetIntVecConfig(uint8_t cd, uint16_t iv)
{
    LOG_NRM("Setting coalescing disable value (CD): 0x%02X", cd);
    LOG_NRM("Setting interrupt vector (IV): 0x%04X", iv);

    uint32_t dw11 = cd & 0x1;
    dw11 <<= 16;
    dw11 |= iv;
    SetDword(dw11, 11);
}


uint32_t
SetFeatures::GetIntVecConfig() const
{
    LOG_NRM("Getting interrupt coalescing (TIME|THR)");
    return GetDword(11);
}


void
SetFeatures::SetAsyncEventConfig(uint16_t critWarn)
{
    LOG_NRM("Setting critical warning bits for "
        "(SMART/Health critical warnings): 0x%04X", critWarn);

    uint32_t dw11 = critWarn;
    SetDword(dw11, 11);
}


uint16_t
SetFeatures::GetAsyncEventConfig() const
{
    LOG_NRM("Getting critical warning bit vector SMART/Health critical warns)");
    return GetWord(11, 0);
}


void
SetFeatures::SetSWProgressMarker(uint8_t pbslc)
{
    LOG_NRM("Setting software progress marker (PBSLC): 0x%02X", pbslc);

    uint32_t dw11 = pbslc & 0xFF;
    SetDword(dw11, 11);
}


uint8_t
SetFeatures::GetSWProgressMarker() const
{
    LOG_NRM("Getting software progress marker (PBSLC)");
    return GetByte(11, 0);
}
