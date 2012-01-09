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

#include <math.h>
#include "iocq.h"
#include "globals.h"

SharedIOCQPtr IOCQ::NullIOCQPtr;
const uint16_t IOCQ::COMMON_ELEMENT_SIZE = 16;
const uint8_t  IOCQ::COMMON_ELEMENT_SIZE_PWR_OF_2 = 4;


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
IOCQ::Init(uint16_t qId, uint16_t numEntries, bool irqEnabled,
    uint16_t irqVec)
{
    uint8_t entrySize;
    uint64_t work;

    if (gRegisters->Read(CTLSPC_CAP, work) == false) {
        LOG_ERR("Unable to determine MQES");
        throw exception();
    }
    // Warn if doing something that looks suspicious
    work &= CAP_MQES;
    if (work < (uint64_t)numEntries) {
        LOG_WARN("Creating Q with %d entries, but DUT only allows %d",
            numEntries, (uint32_t)work);
    }

    if (gCtrlrConfig->GetIOCQES(entrySize) == false) {
        LOG_ERR("Unable to learn IOCQ entry size");
        throw exception();
    }
    CQ::Init(qId, (uint16_t)pow(2, entrySize), numEntries, irqEnabled, irqVec);
}


void
IOCQ::Init(uint16_t qId, uint16_t numEntries,
    const SharedMemBufferPtr memBuffer, bool irqEnabled, uint16_t irqVec)
{
    uint8_t entrySize;
    uint64_t work;

    if (gRegisters->Read(CTLSPC_CAP, work) == false) {
        LOG_ERR("Unable to determine MQES");
        throw exception();
    }
    // Warn if doing something that looks suspicious
    work &= CAP_MQES;
    if (work < (uint64_t)numEntries) {
        LOG_WARN("Creating Q with %d entries, but DUT only allows %d",
            numEntries, (uint32_t)work);
    }

    if (gCtrlrConfig->GetIOCQES(entrySize) == false) {
        LOG_ERR("Unable to learn IOCQ entry size");
        throw exception();
    }
    CQ::Init(qId, (uint16_t)pow(2, entrySize), numEntries, memBuffer,
        irqEnabled, irqVec);
}
