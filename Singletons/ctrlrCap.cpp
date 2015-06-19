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

#include "ctrlrCap.h"
#include "globals.h"
#include "../Exception/frmwkEx.h"

bool CtrlrCap::mInstanceFlag = false;
CtrlrCap *CtrlrCap::mSingleton = NULL;
CtrlrCap *CtrlrCap::GetInstance(SpecRev specRev)
{
    LOG_NRM("Instantiating global CtrlrCap object");
    if(mInstanceFlag == false) {
        mSingleton = new CtrlrCap(specRev);
        mInstanceFlag = true;
    }
    return mSingleton;
}
void CtrlrCap::KillInstance()
{
    if(mInstanceFlag) {
        mInstanceFlag = false;
        delete mSingleton;
        mSingleton = NULL;
    }
}


CtrlrCap::CtrlrCap(SpecRev specRev)
{
    mSpecRev = specRev;

    readSuccess = gRegisters->Read(CTLSPC_CAP, mRegCAP);
}


CtrlrCap::~CtrlrCap()
{
    mInstanceFlag = false;
}

bool CtrlrCap::ReadRegCAP(uint64_t &regVal)
{
    if (readSuccess)
        regVal = mRegCAP;
    return readSuccess;
}

bool CtrlrCap::GetRESVD0(uint8_t &value)
{
    bool retVal;
    uint16_t tmp;
    retVal = GetRegValue(tmp, CAP_RES0, CAP_SH_RES0);
    value = (uint8_t)tmp;
    LOG_NRM("Reading CAP.RESVD0 = 0x%02X", value);
    return retVal;
}

bool CtrlrCap::GetMPSMAX(uint8_t &value)
{
    bool retVal;
    uint16_t tmp;
    retVal = GetRegValue(tmp, CAP_MPSMAX, CAP_SH_MPSMAX);
    value = (uint8_t)tmp;
    LOG_NRM("Reading CAP.MPSMAX = 0x%X", value);
    return retVal;
}

bool CtrlrCap::GetMPSMIN(uint8_t &value)
{
    bool retVal;
    uint16_t tmp;
    retVal = GetRegValue(tmp, CAP_MPSMIN, CAP_SH_MPSMIN);
    value = (uint8_t)tmp;
    LOG_NRM("Reading CAP.MPSMIN = 0x%X", value);
    return retVal;
}

bool CtrlrCap::GetRESVD1(uint8_t &value)
{
    bool retVal;
    uint16_t tmp;

    if (mSpecRev == SPECREV_10b)
        retVal = GetRegValue(tmp, CAP_RES1, CAP_SH_RES1);
    else
        retVal = GetRegValue(tmp, CAP_RES1_r11, CAP_SH_RES1_r11);

    value = (uint8_t)tmp;
    LOG_NRM("Reading CAP.RESVD1 = 0x%X", value);
    return retVal;
}

bool CtrlrCap::GetCSS(uint8_t &value)
{
    bool retVal;
    uint16_t tmp;

    if (mSpecRev == SPECREV_10b)
        retVal = GetRegValue(tmp, CAP_CSS, CAP_SH_CSS);
    else
        retVal = GetRegValue(tmp, CAP_CSS_r11, CAP_SH_CSS);

    value = (uint8_t)tmp;
    LOG_NRM("Reading CAP.CSS = 0x%02X", value);
    return retVal;
}

bool CtrlrCap::GetNVMCSS(bool &value)
{
    bool retVal;
    uint8_t tmp;
    retVal = GetCSS(tmp);
    value = (tmp & CSSBits::CAP_CSS_NVMCS) != 0;
    return retVal;
}

bool CtrlrCap::GetRESVD2(uint8_t &value)
{
    bool retVal;
    uint16_t tmp;

    if (mSpecRev == SPECREV_10b)
        retVal = GetRegValue(tmp, CAP_RES2, CAP_SH_RES2);
    else
        throw FrmwkEx(HERE, "Field does not exist in current SpecRev");

    retVal = GetRegValue(tmp, CAP_RES2, CAP_SH_RES2);
    value = (uint8_t)tmp;
    LOG_NRM("Reading CAP.RESVD2 = 0x%X", value);
    return retVal;
}

bool CtrlrCap::GetNSSRS(bool &value)
{
    bool retVal;
    uint16_t tmp;
    retVal = GetRegValue(tmp, CAP_NSSRS, CAP_SH_NSSRS);
    value = (tmp != 0);
    LOG_NRM("Reading CAP.NSSRS = 0x%hX", tmp);
    return retVal;
}

bool CtrlrCap::GetDSTRD(uint8_t &value)
{
    bool retVal;
    uint16_t tmp;
    retVal = GetRegValue(tmp, CAP_DSTRD, CAP_SH_DSTRD);
    value = (uint8_t)tmp;
    LOG_NRM("Reading CAP.DSTRD = 0x%X", value);
    return retVal;
}

bool CtrlrCap::GetTO(uint8_t &value)
{
    bool retVal;
    uint16_t tmp;
    retVal = GetRegValue(tmp, CAP_TO, CAP_SH_TO);
    value = (uint8_t)tmp;
    LOG_NRM("Reading CAP.TO = 0x%02X", value);
    return retVal;
}

bool CtrlrCap::GetRESVD3(uint8_t &value)
{
    bool retVal;
    uint16_t tmp;
    retVal = GetRegValue(tmp, CAP_RES3, CAP_SH_RES3);
    value = (uint8_t)tmp;
    LOG_NRM("Reading CAP.RESVD3 = 0x%X", value);
    return retVal;
}

bool CtrlrCap::GetAMS(uint8_t &value)
{
    bool retVal;
    uint16_t tmp;
    retVal = GetRegValue(tmp, CAP_AMS, CAP_SH_AMS);
    value = (uint8_t)tmp;
    LOG_NRM("Reading CAP.AMS = 0x%X", value);
    return retVal;
}

bool CtrlrCap::GetCQR(bool &value)
{
    bool retVal;
    uint16_t tmp;
    retVal = GetRegValue(tmp, CAP_CQR, CAP_SH_CQR);
    value = (tmp != 0);
    LOG_NRM("Reading CAP.CQR = 0x%X", tmp);
    return retVal;
}

bool CtrlrCap::GetMQES(uint16_t &value)
{
    bool retVal;
    retVal = GetRegValue(value, CAP_MQES, CAP_SH_MQES);
    LOG_NRM("Reading CAP.MQES = 0x%04X", value);
    return retVal;
}


bool
CtrlrCap::GetRegValue(uint16_t &value, uint64_t regMask,
            uint8_t bitShift)
{
    uint64_t regVal;
    bool retVal = ReadRegCAP(regVal);
    value = (uint16_t)((regVal & regMask) >> bitShift);
    return retVal;
}

void
CtrlrCap::prettyPrint(void)
{
    const CtlSpcType *ctlMetrics = gRegisters->GetCtlMetrics();
    LOG_NRM("%s", gRegisters->FormatRegister(ctlMetrics[CTLSPC_CAP].size,
            ctlMetrics[CTLSPC_CAP].desc, mRegCAP).c_str());
}
