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

#include "iocq.h"
#include "globals.h"

SharedIOCQPtr IOCQ::NullIOCQPtr;


IOCQ::IOCQ() : CQ(0, Trackable::OBJTYPE_FENCE)
{
    // This constructor will throw
}


IOCQ::IOCQ(int fd) : CQ(fd, Trackable::OBJ_IOCQ)
{
}


IOCQ::~IOCQ()
{
}


void
IOCQ::Init(uint16_t qId, uint32_t numEntries, bool irqEnabled, uint16_t irqVec)
{
    uint8_t entrySize;
    uint64_t work;


    LOG_NRM("IOCQ::Init (qId,numEntry,irqEnable,irqVec) = (%d,%d,%d,%d)",
        qId, numEntries, irqEnabled, irqVec);

    if (gCtrlrConfig->GetIOCQES(entrySize) == false)
        throw FrmwkEx(HERE, "Unable to learn IOCQ entry size");

    // Nothing to gain by specifying an element size which the DUT doesn't
    // support, the outcome is undefined, might succeed in crashing the kernel
    ConstSharedIdentifyPtr idCtrlrCap = gInformative->GetIdentifyCmdCtrlr();
    uint64_t value = idCtrlrCap->GetValue(IDCTRLRCAP_CQES);
    uint8_t maxElemSize = (uint8_t)((value >> 4) & 0x0f);
    uint8_t minElemSize = (uint8_t)((value >> 0) & 0x0f);
    if ((entrySize < minElemSize) || (entrySize > maxElemSize)) {
        throw FrmwkEx(HERE, "Reg CC.IOCQES yields a bad element size: 0x%04X",
            (1 << entrySize));
    }

    // Detect if doing something that looks suspicious/incorrect/illegal
    if (gRegisters->Read(CTLSPC_CAP, work) == false)
        throw FrmwkEx(HERE, "Unable to determine MQES");

    work &= CAP_MQES;
    work += 1;      // convert to 1-based
    if ((work + 1) < (uint64_t)numEntries) {
        LOG_WARN("Creating Q with %d entries, but DUT only allows %d",
            numEntries, (uint32_t)(work + 1));
    }

    CQ::Init(qId, (1 << entrySize), numEntries, irqEnabled, irqVec);
}


void
IOCQ::Init(uint16_t qId, uint32_t numEntries,
    const SharedMemBufferPtr memBuffer, bool irqEnabled, uint16_t irqVec)
{
    uint8_t entrySize;
    uint64_t work;


    LOG_NRM("IOSQ::Init (qId,numEntry,irqEnable,irqVec) = (%d,%d,%d,%d)",
        qId, numEntries, irqEnabled, irqVec);
    if (gRegisters->Read(CTLSPC_CAP, work) == false)
        throw FrmwkEx(HERE, "Unable to determine MQES");

    // Detect if doing something that looks suspicious/incorrect/illegal
    work &= CAP_MQES;
    work += 1;      // convert to 1-based
    if ((work + 1) < (uint64_t)numEntries) {
        LOG_WARN("Creating Q with %d entries, but DUT only allows %d",
            numEntries, (uint32_t)(work + 1));
    }

    if (gCtrlrConfig->GetIOCQES(entrySize) == false)
        throw FrmwkEx(HERE, "Unable to learn IOCQ entry size");

    // Nothing to gain by specifying an element size which the DUT doesn't
    // support, the outcome is undefined, might succeed in crashing the kernel
    ConstSharedIdentifyPtr idCtrlrCap = gInformative->GetIdentifyCmdCtrlr();
    uint64_t value = idCtrlrCap->GetValue(IDCTRLRCAP_CQES);
    uint8_t maxElemSize = (uint8_t)((value >> 4) & 0x0f);
    uint8_t minElemSize = (uint8_t)((value >> 0) & 0x0f);
    if ((entrySize < minElemSize) || (entrySize > maxElemSize)) {
        throw FrmwkEx(HERE, "Reg CC.IOCQES yields a bad element size: 0x%04X",
            (1 << entrySize));
    }
    CQ::Init(qId, (1 << entrySize), numEntries, memBuffer,
        irqEnabled, irqVec);
}
