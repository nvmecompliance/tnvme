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

#include "ce.h"
#include "globals.h"


// Contains details about every CE status field
#define ZZ(a, b, c, d)      { b, c, d },
CEStatType ProcessCE::mCEStatMetrics[] =
{
    CESTAT_TABLE
};
#undef ZZ


ProcessCE::ProcessCE()
{
}


ProcessCE::~ProcessCE()
{
}


void
ProcessCE::ValidateStatus(union CE &ce)
{
    if (LogStatus(ce) == false)
        throw exception();
}


bool
ProcessCE::LogStatus(union CE &ce)
{
    LOG_NRM("Decode: CE.status=0x%04X", ce.n.SF.t.status);
    if (ce.n.SF.b.DNR)
        LOG_NRM("  DNR = 0x%01X (do not retry)", ce.n.SF.b.DNR);
    if (ce.n.SF.b.M)
        LOG_NRM("  M   = 0x%01X (more status available)", ce.n.SF.b.M);

    switch (ce.n.SF.b.SCT) {
    case SCT_GENERIC:
        LOG_NRM("  SCT = 0x%01X (generic cmd status)", ce.n.SF.b.SCT);
        break;
    case SCT_CMD:
        LOG_NRM("  SCT = 0x%01X (cmd specific errors)", ce.n.SF.b.SCT);
        break;
    case SCT_MEDIA:
        LOG_NRM("  SCT = 0x%01X (media errors)", ce.n.SF.b.SCT);
        break;
    case SCT_VENDOR:
        LOG_NRM("  SCT = 0x%01X (vendor specific)", ce.n.SF.b.SCT);
        break;
    default:
        LOG_ERR("  SCT = 0x%01X (??? unknown/undefined/illegal)", ce.n.SF.b.SCT);
        return false;
    }

    // If this is vendor specific, then we can't possibly know the description
    if (ce.n.SF.b.SCT == SCT_VENDOR) {
        LOG_NRM("  SC  = 0x%02X (Vendor specific)", ce.n.SF.b.SC);
    } else {    // We should be able to lookup the spec defined error
        unsigned int i = 0;
        for ( ; i < (sizeof(mCEStatMetrics) / sizeof(mCEStatMetrics[0])); i++) {

            if (mCEStatMetrics[i].sct == ce.n.SF.b.SCT) {
                if (mCEStatMetrics[i].sc == ce.n.SF.b.SC) {
                    LOG_NRM("  SC  = 0x%02X (%s)", ce.n.SF.b.SC,
                        mCEStatMetrics[ce.n.SF.b.SC].desc);
                    break;
                }
            }
        }

        if (i >= (sizeof(mCEStatMetrics) / sizeof(mCEStatMetrics[0]))) {
            LOG_ERR("Detected unknown combo: SCT:SC = 0x%01X,0x%02X",
                ce.n.SF.b.SCT, ce.n.SF.b.SC);
            return false;
        }
    }

    if (ce.n.SF.t.status != 0) {
        LOG_ERR("Detected unsuccessful CE...");
        LOG_NRM("  DWORD0: 0x%08X", ce.t.dw0);
        LOG_NRM("  DWORD1: 0x%08X", ce.t.dw1);
        LOG_NRM("  DWORD2: 0x%08X", ce.t.dw2);
        LOG_NRM("  DWORD3: 0x%08X", ce.t.dw3);
        return false;
    }

    return true;
}
