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

#ifndef _CE_H_
#define _CE_H_

#include <stdint.h>
#include <vector>
#include <string>
#include "ceDefs.h"

using namespace std;


struct StatbyBits {
    uint16_t P        : 1;      // phase bit
    uint16_t SC       : 8;      // status code
    uint16_t SCT      : 3;      // status code type
    uint16_t reserved : 2;
    uint16_t M        : 1;      // more
    uint16_t DNR      : 1;      // do not retry
} __attribute__((__packed__));

struct StatbyType {
    uint16_t P        : 1;      // phase bit
    uint16_t status   : 15;     // status field
} __attribute__((__packed__));

union StatField {
    struct StatbyBits   b;
    struct StatbyType   t;
};

struct CEbyType {
    uint32_t dw0;
    uint32_t dw1;
    uint32_t dw2;
    uint32_t dw3;
} __attribute__((__packed__));

struct CEbyName {
    uint32_t  cmdSpec;
    uint32_t  reserved;
    uint16_t  SQHD;
    uint16_t  SQID;
    uint16_t  CID;
    StatField SF;
} __attribute__((__packed__));

/**
 * Completion Element (CE) definition.
 * @note: For convenient methods to log/dump/peek; refer to class CQ
 */
union CE {
    struct CEbyType t;
    struct CEbyName n;
};


/**
* This class is meant not be instantiated because it should only ever contain
* static members. These helper methods/functions can be viewed as wrappers to
* perform common, repetitious tasks to act upon a CE object.
*
* @note This class may throw exceptions, please see comment within specific
*       methods.
*/
class ProcessCE
{
public:
    ProcessCE();
    virtual ~ProcessCE();

    /**
     * Peeks at the status field of the CE, logs a human readable description
     * of the status and returns when (status == success), otherwise it
     * will throw.
     * @note This method will throw when (status != 0)
     * @param ce Pass the CE to perform the interrogation against
     */
    static void ValidateStatus(union CE &ce);

    /**
     * Indicate the status of the CE in the log file, i.e. stderr.
     * @note This method may throw
     * @param ce Pass the CE to perform the interrogation against
     * @return true if (status == success), otherwise false.
     */
    static bool LogStatus(union CE &ce);

    /**
     * Decode the status of the CE into human readable strings.
     * @note This method may throw
     * @param ce Pass the CE to perform the interrogation against
     * @param desc Returns an array of decoded strings
     * @return true if (status == success), otherwise false.
     */
    static bool DecodeStatus(union CE &ce, vector<string> &desc);

private:
    /// Contains details about every CE status field
    static CEStatType mCEStatMetrics[];
};


#endif
