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
#include "../Utils/buffers.h"


SetFeatures::SetFeatures() : BaseFeatures(0, Trackable::OBJTYPE_FENCE)
{
    // This constructor will throw
}


SetFeatures::SetFeatures(int fd) : BaseFeatures(fd, Trackable::OBJ_SETFEATURES)
{
    Init(0x09, DATADIR_TO_DEVICE);
}


SetFeatures::~SetFeatures()
{
}


void
SetFeatures::Dump(LogFilename filename, string fileHdr) const
{
    Cmd::Dump(filename, fileHdr);
    PrpData::Dump(filename, "Payload contents:");
    MetaData::Dump(filename, "Meta data contents:");
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

