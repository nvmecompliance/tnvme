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

#ifndef _TESTDESCRIBE_H_
#define _TESTDESCRIBE_H_

#include <string>
#include "tnvme.h"


/**
 * This class is meant to document/describe the each test case to completeness.
 *
 * @note This class will not throw exceptions.
 */
class TestDescribe
{
public:
    TestDescribe();
    virtual ~TestDescribe();

    /**
     * Set a 1-line comment briefly describing the purpose of a
     * test case.
     * @param desc Pass a 1-line comment
     */
    void SetShort(string desc);

    /**
     * Set a multi-line comment verbosely describing the purpose of a test case.
     * Any embedded CR or LF are removed and the string is reformatted to
     * conform to the max chars per line requirement, thus alleviating the
     * formatting burden from the programmer and to catch and correct mistakes.
     * @param desc Pass a multi-line comment
     */
    void SetLong(string desc);

    /**
     * Set a 1-line comment describing the targeted compliance
     * of a test case. Example: "rev 1.0a, section 4"
     * @param desc Pass a 1-line comment
     */
    void SetCompliance(string desc);

    /**
     * @param limit80Chars Pass whether to format the long paragraph to 80
     *          character column displays.
     * @param indent Pass num of spaces to indent the long paragraph. The
     *              entire paragraph is word wrapped at 80 chars with each new
     *              line indented according to this param.
     */
    string GetLong(bool limit80Chars, size_t indent);

    /// No formatting or indentation for these methods
    string GetLong()        { return mLongDesc; }
    string GetShort()       { return mShortDesc; }
    string GetCompliance()  { return mCompliance; }


private:
    string mShortDesc;
    string mLongDesc;
    string mCompliance;
};


#endif
