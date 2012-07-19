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

#include <stdio.h>
#include "ce.h"
#include "globals.h"
#include "../Cmds/getLogPage.h"
#include "../Utils/io.h"


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
ProcessCE::Validate(union CE &ce, CEStat status)
{
    LogStatus(ce);

    if (ValidatePeek(ce, status) == false)
        throw FrmwkEx(HERE);
}


void
ProcessCE::ValidateDetailed(union CE &ce, StatbyBits &status)
{
    LogStatus(ce);

    if ((status.DNR != ce.n.SF.b.DNR) ||
        (status.M   != ce.n.SF.b.M) ||
        (status.P   != ce.n.SF.b.P) ||
        (status.SC  != ce.n.SF.b.SC) ||
        (status.SCT != ce.n.SF.b.SCT)) {

        throw FrmwkEx(HERE,
            "Expected (DNR:M:P:SCT:SC) 0x%02X:0x%01X:0x%01X:0x%02X:0x%02X, "
            "but detected x%02X:0x%01X:0x%01X:0x%02X:0x%02X",
            status.DNR, status.M, status.P, status.SC, status.SCT,
            ce.n.SF.b.DNR, ce.n.SF.b.M, ce.n.SF.b.P, ce.n.SF.b.SCT,
            ce.n.SF.b.SC);
    } else if ((ce.n.SF.b.SCT == mCEStatMetrics[CESTAT_SUCCESS].sct) &&
               (ce.n.SF.b.SC  == mCEStatMetrics[CESTAT_SUCCESS].sc)) {

        if (ce.n.SF.b.M) {
            throw FrmwkEx(HERE,
                "Illegal 'M' bit set while CE indicates success");
        }
    }
}


bool
ProcessCE::ValidatePeek(union CE &ce, CEStat status)
{
    LogStatus(ce);

    if ((ce.n.SF.b.SCT != mCEStatMetrics[status].sct) ||
        (ce.n.SF.b.SC  != mCEStatMetrics[status].sc)) {
        LOG_ERR("Expected (SCT:SC) 0x%02X:0x%02X, but detected 0x%02X:0x%02X",
            mCEStatMetrics[status].sct, mCEStatMetrics[status].sc,
            ce.n.SF.b.SCT, ce.n.SF.b.SC);
        return false;
    } else if ((ce.n.SF.b.SCT == mCEStatMetrics[CESTAT_SUCCESS].sct) &&
               (ce.n.SF.b.SC  == mCEStatMetrics[CESTAT_SUCCESS].sc)) {

        if (ce.n.SF.b.M) {
            LOG_ERR("Illegal 'M' bit set while CE indicates success");
            return false;
        }
    }
    return true;
}


void
ProcessCE::LogStatus(union CE &ce)
{
    vector<string> desc;

    DecodeStatus(ce, desc);
    for (size_t i = 0; i < desc.size(); i++ )
        LOG_NRM("%s", desc[i].c_str());
}


void
ProcessCE::DecodeStatus(union CE &ce, vector<string> &desc)
{
    char work[256];

    desc.clear();
    snprintf(work, sizeof(work), "Decode: CE.status=0x%04X", ce.n.SF.t.status);
    desc.push_back(work);


    if (ce.n.SF.b.DNR) {
        snprintf(work, sizeof(work),
            "  DNR = 0x%01X (do not retry)", ce.n.SF.b.DNR);
        desc.push_back(work);
    }
    if (ce.n.SF.b.M) {
        snprintf(work, sizeof(work),
            "  M   = 0x%01X (more status available)", ce.n.SF.b.M);
        desc.push_back(work);
    }


    switch (ce.n.SF.b.SCT) {
    case SCT_GENERIC:
        snprintf(work, sizeof(work),
            "  SCT = 0x%01X  (generic cmd status)", ce.n.SF.b.SCT);
        break;
    case SCT_CMD:
        snprintf(work, sizeof(work),
            "  SCT = 0x%01X  (cmd specific errors)", ce.n.SF.b.SCT);
        break;
    case SCT_MEDIA:
        snprintf(work, sizeof(work),
            "  SCT = 0x%01X  (media errors)", ce.n.SF.b.SCT);
        break;
    case SCT_VENDOR:
        snprintf(work, sizeof(work),
            "  SCT = 0x%01X  (vendor specific)", ce.n.SF.b.SCT);
        break;
    default:
        snprintf(work, sizeof(work),
            "  SCT = 0x%01X  (??? unknown/undefined/illegal)", ce.n.SF.b.SCT);
        desc.push_back(work);
    }
    desc.push_back(work);


    // If this is vendor specific, then we can't possibly know the description
    if (ce.n.SF.b.SCT == SCT_VENDOR) {
        snprintf(work, sizeof(work),
            "  SC  = 0x%02X (Vendor specific)", ce.n.SF.b.SC);
        desc.push_back(work);
    } else {    // We should be able to lookup the spec defined error
        unsigned int i = 0;
        for ( ; i < (sizeof(mCEStatMetrics) / sizeof(mCEStatMetrics[0])); i++) {

            if (mCEStatMetrics[i].sct == ce.n.SF.b.SCT) {
                if (mCEStatMetrics[i].sc == ce.n.SF.b.SC) {
                    snprintf(work, sizeof(work), "  SC  = 0x%02X (%s)",
                        ce.n.SF.b.SC, mCEStatMetrics[i].desc);
                    desc.push_back(work);
                    break;
                }
            }
        }

        if (i >= (sizeof(mCEStatMetrics) / sizeof(mCEStatMetrics[0]))) {
            snprintf(work, sizeof(work),
                "Detected unknown combo: SCT:SC = 0x%01X,0x%02X",
                ce.n.SF.b.SCT, ce.n.SF.b.SC);
            desc.push_back(work);
        }
    }


    if (ce.n.SF.t.status != 0) {
        snprintf(work, sizeof(work), "Detected unsuccessful CE...");
        desc.push_back(work);
        snprintf(work, sizeof(work), "  DWORD0: 0x%08X", ce.t.dw0);
        desc.push_back(work);
        snprintf(work, sizeof(work), "  DWORD1: 0x%08X", ce.t.dw1);
        desc.push_back(work);
        snprintf(work, sizeof(work), "  DWORD2: 0x%08X", ce.t.dw2);
        desc.push_back(work);
        snprintf(work, sizeof(work), "  DWORD3: 0x%08X", ce.t.dw3);
        desc.push_back(work);
    }
}
