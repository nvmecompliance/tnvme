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

struct AsyncEventReqs {
    uint8_t asyncEventType  : 3;
    uint8_t rsvd03          : 5;
    uint8_t asyncEventInfo;
    uint8_t assocLogPage;
    uint8_t rsvd24;
} __attribute__((__packed__));

struct CEbyType {
    uint32_t dw0;
    uint32_t dw1;
    uint32_t dw2;
    uint32_t dw3;
} __attribute__((__packed__));


struct CEbyName {
    union {
        uint32_t cmdSpec;
        struct AsyncEventReqs async;
    };
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
     * of the status and returns when the supplied expected status is what is
     * contained within the supplied CE, otherwise it will throw.
     * @note This method may throw
     * @param ce Pass the CE to perform the interrogation against
     * @param status Pass the expected status to verify with
     */
    static void Validate(union CE &ce, CEStat status = CESTAT_SUCCESS);

    /**
     * Peeks at the status field of the CE, logs a human readable description
     * of the status and returns when the supplied expected status is what is
     * contained within the supplied CE, otherwise it will throw.
     * @note This method may throw
     * @param ce Pass the CE to perform the interrogation against
     * @param status Pass the expected state of the CE to verify with
     */
    static void ValidateDetailed(union CE &ce, StatbyBits &status);

    /**
     * Peeks at the status field of the CE, when the supplied expected status
     * is what is contained with the supplied CE it will return true,
     * otherwise it will return false.
     * @note This method never throws
     * @param ce Pass the CE to perform the interrogation against
     * @param status Pass the expected state of the CE to verify with
     * @return true upon success, otherwise false.
     */
    static bool ValidatePeek(union CE &ce, CEStat status = CESTAT_SUCCESS);

    /**
     * Indicate the status of the CE in the log file, i.e. stderr.
     * @note This method may throw
     */
    static void LogStatus(union CE &ce);

    /**
     * Decode the status of the CE into human readable strings.
     * @note This method may throw
     * @param ce Pass the CE to perform the interrogation against
     * @param desc Returns an array of decoded strings
     */
    static void DecodeStatus(union CE &ce, vector<string> &desc);

private:
    /// Contains details about every CE status field
    static CEStatType mCEStatMetrics[];
};


#endif
