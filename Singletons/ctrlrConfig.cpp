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

#include "ctrlrConfig.h"
#include "globals.h"
#include "../Exception/frmwkEx.h"

const uint16_t CtrlrConfig::MAX_MSI_SINGLE_IRQ_VEC = 0;
const uint16_t CtrlrConfig::MAX_MSI_MULTI_IRQ_VEC = 31;
const uint16_t CtrlrConfig::MAX_MSIX_IRQ_VEC = 2047;
const uint8_t CtrlrConfig::CSS_NVM_CMDSET   = 0x00;


bool CtrlrConfig::mInstanceFlag = false;
CtrlrConfig *CtrlrConfig::mSingleton = NULL;
CtrlrConfig *CtrlrConfig::GetInstance(int fd, SpecRev specRev)
{
    LOG_NRM("Instantiating global CtrlrConfig object");
    if(mInstanceFlag == false) {
        mSingleton = new CtrlrConfig(fd, specRev);
        mInstanceFlag = true;
    }
    return mSingleton;
}
void CtrlrConfig::KillInstance()
{
    if(mInstanceFlag) {
        mInstanceFlag = false;
        delete mSingleton;
        mSingleton = NULL;
    }
}


CtrlrConfig::CtrlrConfig(int fd, SpecRev specRev) :
    SubjectCtrlrState(ST_DISABLE_COMPLETELY)
{
    mFd = fd;
    if (mFd < 0)
        throw FrmwkEx(HERE, "Object created with a bad FD=%d", fd);

    mSpecRev = specRev;

    uint64_t tmp;
    gRegisters->Read(CTLSPC_CAP, tmp);
    mRegCAP = (uint32_t)tmp;
}


CtrlrConfig::~CtrlrConfig()
{
    mInstanceFlag = false;
}


bool
CtrlrConfig::IsMSICapable(bool &capable, uint16_t &numIrqs)
{
    uint64_t value;
    numIrqs = 0;
    capable = false;
    const vector<PciCapabilities> *pciCap = gRegisters->GetPciCapabilities();

    for (size_t i = 0; i < pciCap->size(); i++) {
        if (pciCap->at(i) == PCICAP_MSICAP) {
            if (gRegisters->Read(PCISPC_MC, value) == false) {
                LOG_ERR("Unable to determine IRQ capability");
                return false;
            }
            uint16_t work = (uint16_t)((value & MC_MMC) >> 1);
            capable = true;
            numIrqs = work;
            break;
        }
    }
    LOG_NRM("Detected %d MSI IRQ(s) supported", numIrqs);
    return true;
}


bool
CtrlrConfig::IsMSIXCapable(bool &capable, uint16_t &numIrqs)
{
    uint64_t value;
    numIrqs = 0;
    capable = false;
    const vector<PciCapabilities> *pciCap = gRegisters->GetPciCapabilities();

    for (size_t i = 0; i < pciCap->size(); i++) {
        if (pciCap->at(i) == PCICAP_MSIXCAP) {
            if (gRegisters->Read(PCISPC_MXC, value) == false) {
                LOG_ERR("Unable to determine IRQ capability");
                return false;
            }
            uint16_t work = (uint16_t)((value & MXC_TS) + 1);
            capable = true;
            numIrqs = work;
            break;
        }
    }
    LOG_NRM("Detected %d MSI-X IRQ(s) supported", numIrqs);
    return true;
}


bool
CtrlrConfig::GetIrqScheme(enum nvme_irq_type &irq, uint16_t &numIrqs)
{
    public_metrics_dev state;

    if (ioctl(mFd, NVME_IOCTL_GET_DEVICE_METRICS, &state) < 0) {
        LOG_ERR("Unable to get IRQ scheme");
        return false;
    }

    irq = state.irq_active.irq_type;
    numIrqs = state.irq_active.num_irqs;

    string irqDesc;
    if (DecodeIrqScheme(irq, irqDesc) == false) {
        LOG_ERR("%s", irqDesc.c_str());
        return false;
    }
    LOG_NRM("Getting IRQ state: %d IRQ(s) of %s", numIrqs, irqDesc.c_str());
    return true;
}


bool
CtrlrConfig::SetIrqScheme(enum nvme_irq_type newIrq, uint16_t numIrqs)
{
    if (IsStateEnabled()) {
        LOG_ERR("The NVMe must be disabled in order to change the IRQ scheme");
        return false;
    }

    string irqDesc;
    if (DecodeIrqScheme(newIrq, irqDesc) == false) {
        LOG_ERR("Unable to decode IRQ scheme");
        return false;
    }
    LOG_NRM("Setting IRQ state: %d IRQ(s) of %s", numIrqs, irqDesc.c_str());

    struct interrupts state;
    state.irq_type = newIrq;
    state.num_irqs = numIrqs;
    if (ioctl(mFd, NVME_IOCTL_SET_IRQ, &state) < 0) {
        LOG_ERR("%s", irqDesc.c_str());
        return false;
    }
    return true;
}


bool
CtrlrConfig::IrqsEnabled()
{
    enum nvme_irq_type irq;
    uint16_t numIrqs;

    if (GetIrqScheme(irq, numIrqs) == false)
        return false;   // no, it is disabled

    switch (irq) {
    case INT_MSI_SINGLE:
    case INT_MSI_MULTI:
    case INT_MSIX:
         return true;   // yes, it is enabled
    case INT_NONE:
    default:
         return false;  // no, it is disabled
    }
}


bool
CtrlrConfig::DecodeIrqScheme(enum nvme_irq_type newIrq, string &desc)
{
    switch (newIrq) {
    case INT_MSI_SINGLE:    desc = "MSI-single IRQ scheme";     return true;
    case INT_MSI_MULTI:     desc = "MSI-multi IRQ scheme";      return true;
    case INT_MSIX:          desc = "MSI-X IRQ scheme";          return true;
    case INT_NONE:          desc = "no IRQ scheme";             return true;
    default:                desc = "unknown IRQ scheme";        return false;
    }
}


bool
CtrlrConfig::IsStateEnabled()
{
    uint64_t tmp = 0;
    if (gRegisters->Read(CTLSPC_CSTS, tmp))
        return (tmp & CSTS_RDY);
    return false;
}


bool
CtrlrConfig::SetState(enum nvme_state state)
{
    string toState;

    switch (state) {
    case ST_ENABLE:
        // Always conform to page size of the active architecture
        if (SetMPS() == false)
            return false;
        toState = "Enabling";
        break;
    case ST_DISABLE:
        toState = "Disabling";
        break;
    case ST_DISABLE_COMPLETELY:
        toState = "Disabling completely";
        break;
    default:
        throw FrmwkEx(HERE, "Illegal state detected = %d", state);
    }

    LOG_NRM("%s the NVME device", toState.c_str());
    if (ioctl(mFd, NVME_IOCTL_DEVICE_STATE, state) < 0) {
        LOG_ERR("Could not set state, currently %s",
            IsStateEnabled() ? "enabled" : "disabled");
        LOG_NRM("dnvme waits a TO period for CC.RDY to indicate ready" );
        return false;
    }

    // The state of the ctrlr is important to many objects
    Notify(state);

    return true;
}


bool
CtrlrConfig::ReadRegCC(uint32_t &regVal)
{
    uint64_t tmp;
    bool retVal = gRegisters->Read(CTLSPC_CC, tmp);
    regVal  = (uint32_t)tmp;
    return retVal;
}


bool
CtrlrConfig::WriteRegCC(uint32_t regVal)
{
    uint64_t tmp =  regVal;
    bool retVal = gRegisters->Write(CTLSPC_CC, tmp);
    return retVal;
}


bool
CtrlrConfig::GetRegValue(uint8_t &value, uint32_t regMask, uint8_t bitShift)
{
    uint32_t regVal;
    bool retVal = ReadRegCC(regVal);
    value = (uint8_t)((regVal & regMask) >> bitShift);
    return retVal;
}


bool
CtrlrConfig::SetRegValue(uint8_t value, uint8_t valueMask, uint64_t regMask,
    uint8_t bitShift)
{
    if (value & ~valueMask) {
        LOG_ERR("Parameter value is to large: 0x%02X", value);
        return false;
    }

    uint32_t regVal;
    if (ReadRegCC(regVal) == false)
        return false;
    regVal &= ~regMask;
    regVal |= ((uint32_t)value << bitShift);
    return WriteRegCC(regVal);
}


bool
CtrlrConfig::GetIOCQES(uint8_t &value)
{
    bool retVal;
    retVal = GetRegValue(value, CC_IOCQES, 20);
    LOG_NRM("Reading CC.IOCQES = 0x%02X; (2^%d) = %d", value, value,
        (1 << value));
    return retVal;
}


bool
CtrlrConfig::SetIOCQES(uint8_t value)
{
    LOG_NRM("Writing CC.IOCQES = 0x%02X; (2^%d) = %d", value, value,
        (1 << value));
    return SetRegValue(value, 0x0f, CC_IOCQES, 20);
}


bool
CtrlrConfig::GetIOSQES(uint8_t &value)
{
    bool retVal;
    retVal = GetRegValue(value, CC_IOSQES, 16);
    LOG_NRM("Reading CC.IOSQES = 0x%02X; (2^%d) = %d", value, value,
        (1 << value));
    return retVal;
}


bool
CtrlrConfig::SetIOSQES(uint8_t value)
{
    LOG_NRM("Writing CC.IOSQES = 0x%02X; (2^%d) = %d", value, value,
        (1 << value));
    return SetRegValue(value, 0x0f, CC_IOSQES, 16);
}


bool
CtrlrConfig::GetSHN(uint8_t &value)
{
    bool retVal;
    retVal = GetRegValue(value, CC_SHN, 14);
    LOG_NRM("Reading CC.SHN = 0x%02X", value);
    return retVal;
}


bool
CtrlrConfig::SetSHN(uint8_t value)
{
    LOG_NRM("Writing CC.SHN = 0x%02X", value);
    return SetRegValue(value, 0x03, CC_SHN, 14);
}


bool
CtrlrConfig::GetAMS(uint8_t &value)
{
    bool retVal;
    retVal = GetRegValue(value, CC_AMS, 11);
    LOG_NRM("Reading CC.AMS = 0x%02X", value);
    return retVal;
}


bool
CtrlrConfig::SetAMS(uint8_t value)
{
    LOG_NRM("Writing CC.AMS = 0x%02X", value);
    return SetRegValue(value, 0x07, CC_AMS, 11);
}


bool
CtrlrConfig::GetMPS(uint8_t &value)
{
    bool retVal;
    retVal = GetRegValue(value, CC_MPS, 7);
    LOG_NRM("Reading CC.MPS = 0x%02X", value);
    return retVal;
}


bool
CtrlrConfig::SetMPS()
{
    switch (sysconf(_SC_PAGESIZE)) {
    case 4096:
        LOG_NRM("Writing CC.MPS for a 4096 byte page size");
        return SetRegValue(0, 0x0f, CC_MPS, 7);
    default:
        LOG_ERR("Kernel reporting unsupported page size: 0x%08lX",
            sysconf(_SC_PAGESIZE));
        return false;
    }
}


bool
CtrlrConfig::GetCSS(uint8_t &value)
{
    bool retVal;
    retVal = GetRegValue(value, CC_CSS, 4);
    LOG_NRM("Reading CC.CSS = 0x%02X", value);
    return retVal;
}


bool
CtrlrConfig::SetCSS(uint8_t value)
{
    LOG_NRM("Writing CC.CSS = 0x%02X", value);
    return SetRegValue(value, 0x07, CC_CSS, 4);
}


