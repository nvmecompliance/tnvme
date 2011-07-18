#include <stdarg.h>
#include "tnvme.h"
#include "testDescribe.h"


TestDescribe::TestDescribe()
{
    // 66 chars allowed:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
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

    for (size_t i = 0; i < indent; i++)
        indentation += " ";

    if (limit80Chars) {
        startSpaceIdx = 0;
        spaceIdx = (mLongDesc.length() > (80 - indent)) ?
            (80 - indent) : mLongDesc.length();
        spaceIdx -= 1;      // make up for 0-based indexing

        while (startSpaceIdx < spaceIdx) {
            spaceIdx = mLongDesc.find_last_of(' ', spaceIdx);

            // Returning results identical to the last implies this is last iter
            if ((spaceIdx + 1) == startSpaceIdx)
                spaceIdx = (mLongDesc.length() - 1);

            work += indentation;
            work += mLongDesc.substr(startSpaceIdx, (spaceIdx - startSpaceIdx));
            work += "\n";

            startSpaceIdx = spaceIdx + 1;
            spaceIdx = (mLongDesc.length() > (startSpaceIdx + (80 - indent))) ?
                (startSpaceIdx + (80 - indent)) : mLongDesc.length();
            spaceIdx -= 1;      // make up for 0-based indexing
        }
    } else {
        work += indentation;
        work += mLongDesc;
    }

    return work;
}
