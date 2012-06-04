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

#include "iosq.h"
#include "globals.h"

SharedIOSQPtr IOSQ::NullIOSQPtr;


IOSQ::IOSQ() : SQ(0, Trackable::OBJTYPE_FENCE)
{
    // This constructor will throw
}


IOSQ::IOSQ(int fd) : SQ(fd, Trackable::OBJ_IOSQ)
{
	mPriority = 0;
}


IOSQ::~IOSQ()
{
}


void
IOSQ::Init(uint16_t qId, uint32_t numEntries, uint16_t cqId,
    uint8_t priority)
{
    uint8_t entrySize;
    uint64_t work;


    LOG_NRM("IOSQ::Init (qId,numEntry,cqId,prior) = (%d,%d,%d,%d)",
        qId, numEntries, cqId, priority);

    switch (priority) {
	case 0:
	case 1:
	case 2:
	case 3:
		mPriority = priority;
		break;
	default:
		throw FrmwkEx(HERE, "Illegal priority value, can't fit within 2 bits");
	}

    if (gCtrlrConfig->GetIOSQES(entrySize) == false)
        throw FrmwkEx(HERE, "Unable to learn IOSQ entry size");

    // Nothing to gain by specifying an element size which the DUT doesn't
    // support, the outcome is undefined, might succeed in crashing the kernel
    ConstSharedIdentifyPtr idCtrlrCap = gInformative->GetIdentifyCmdCtrlr();
    uint64_t value = idCtrlrCap->GetValue(IDCTRLRCAP_SQES);
    uint8_t maxElemSize = (uint8_t)((value >> 4) & 0x0f);
    uint8_t minElemSize = (uint8_t)((value >> 0) & 0x0f);
    if ((entrySize < minElemSize) || (entrySize > maxElemSize)) {
        throw FrmwkEx(HERE, "Reg CC.IOSQES yields a bad element size: 0x%04X",
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

    SQ::Init(qId, (1 << entrySize), numEntries, cqId);
}


void
IOSQ::Init(uint16_t qId, uint32_t numEntries,
    const SharedMemBufferPtr memBuffer, uint16_t cqId, uint8_t priority)
{
    uint8_t entrySize;
    uint64_t work;


    LOG_NRM("IOSQ::Init (qId,numEntry,cqId,prior) = (%d,%d,%d,%d)",
        qId, numEntries, cqId, priority);
    if (gRegisters->Read(CTLSPC_CAP, work) == false)
        throw FrmwkEx(HERE, "Unable to determine MQES");

    // Detect if doing something that looks suspicious/incorrect/illegal
    work &= CAP_MQES;
    work += 1;      // convert to 1-based
    if ((work + 1) < (uint64_t)numEntries) {
        LOG_WARN("Creating Q with %d entries, but DUT only allows %d",
            numEntries, (uint32_t)(work + 1));
    }

    switch (priority) {
    case 0:
    case 1:
    case 2:
    case 3:
        mPriority = priority;
        break;
    default:
        throw FrmwkEx(HERE, "Illegal priority value, can't fit within 2 bits");
    }

    if (gCtrlrConfig->GetIOSQES(entrySize) == false)
        throw FrmwkEx(HERE, "Unable to learn IOSQ entry size");

    // Nothing to gain by specifying an element size which the DUT doesn't
    // support, the outcome is undefined, might succeed in crashing the kernel
    ConstSharedIdentifyPtr idCtrlrCap = gInformative->GetIdentifyCmdCtrlr();
    uint64_t value = idCtrlrCap->GetValue(IDCTRLRCAP_SQES);
    uint8_t maxElemSize = (uint8_t)((value >> 4) & 0x0f);
    uint8_t minElemSize = (uint8_t)((value >> 0) & 0x0f);
    if ((entrySize < minElemSize) || (entrySize > maxElemSize)) {
        throw FrmwkEx(HERE, "Reg CC.IOSQES yields a bad element size: 0x%04X",
            (1 << entrySize));
    }
    SQ::Init(qId, (1 << entrySize), numEntries, memBuffer, cqId);
}
