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

#include "acq.h"
#include "globals.h"

SharedACQPtr ACQ::NullACQPtr;
const uint16_t ACQ::IDEAL_ELEMENT_SIZE = 16;


ACQ::ACQ() : CQ(0, Trackable::OBJTYPE_FENCE)
{
    // This constructor will throw
}


ACQ::ACQ(int fd) : CQ(fd, Trackable::OBJ_ACQ)
{
}


ACQ::~ACQ()
{
}


void
ACQ::Init(uint32_t numEntries)
{
    LOG_WARN("Even though ACQ IRQ's enabled, may not be globally");
    CQ::Init(0, IDEAL_ELEMENT_SIZE, numEntries, true, 0);
}
