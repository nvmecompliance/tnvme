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

#include <stdarg.h>
#include "tnvme.h"
#include "testDescribe.h"


TestDescribe::TestDescribe()
{
    // 63 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    mShortDesc =            "Missing short test description";
    mCompliance =           "Missing compliance description";
    // No string size limit for the long description
    mLongDesc  =            "Missing long test description";
}


TestDescribe::~TestDescribe()
{
}


void
TestDescribe::SetShort(string desc)
{
    if (desc.length() > MAX_CHAR_PER_LINE_DESCRIPTION) {
        LOG_ERR("Short description length violation, concatenating \"%s\"",
            desc.c_str());
    }
    mShortDesc = desc.substr(0, MAX_CHAR_PER_LINE_DESCRIPTION);
}


void
TestDescribe::SetLong(string desc)
{
    mLongDesc = "";
    for (size_t i = 0; i < desc.length(); i++) {
        if ((desc[i] == '\r') || (desc[i] == '\n'))
            continue;
        mLongDesc += desc[i];
    }
}


void
TestDescribe::SetCompliance(string desc)
{
    if (desc.length() > MAX_CHAR_PER_LINE_DESCRIPTION)
        LOG_ERR("Compliance description length violation");
    mCompliance = desc.substr(0, MAX_CHAR_PER_LINE_DESCRIPTION);
}


string
TestDescribe::GetLong(bool limit80Chars, size_t indent)
{
    string indentation;
    string work;
    size_t spaceIdx, startSpaceIdx;
    string tmpLongDesc = mLongDesc;

    tmpLongDesc += " ";
    for (size_t i = 0; i < indent; i++)
        indentation += " ";

    if (limit80Chars) {
        startSpaceIdx = 0;
        spaceIdx = (tmpLongDesc.length() > (80 - indent)) ?
            (80 - indent) : tmpLongDesc.length();
        spaceIdx -= 1;      // make up for 0-based indexing

        while (startSpaceIdx < spaceIdx) {
            spaceIdx = tmpLongDesc.find_last_of(' ', spaceIdx);

            // Returning results identical to the last implies this is last iter
            if ((spaceIdx + 1) == startSpaceIdx)
                spaceIdx = (tmpLongDesc.length() - 1);

            work += indentation;
            work += tmpLongDesc.substr(startSpaceIdx,
                (spaceIdx - startSpaceIdx));
            work += "\n";

            startSpaceIdx = spaceIdx + 1;
            spaceIdx =
                (tmpLongDesc.length() > (startSpaceIdx + (80 - indent))) ?
                (startSpaceIdx + (80 - indent)) : tmpLongDesc.length();
            spaceIdx -= 1;      // make up for 0-based indexing
        }
    } else {
        work += indentation;
        work += tmpLongDesc;
    }

    return work;
}
