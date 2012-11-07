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

#ifndef _IRQ_H_
#define _IRQ_H_

#include "tnvme.h"


/**
* This class is meant not be instantiated because it should only ever contain
* static members. These utility functions can be viewed as wrappers to
* perform common, repetitious tasks which avoids coping the same chunks of
* code throughout the framework.
*
* @note This class may throw exceptions, please see comment within specific
*       methods.
*/
class IRQ
{
public:
    IRQ();
    virtual ~IRQ();

    /**
     * Verify the DUT supports the number of desired IRQ's using any IRQ scheme.
     * Search through the fastest IRQ scheme first towards the lease efficient.
     * @note This method may throw
     * @param numIrqsDesire Pass the total number of IRQ's needed
     * @param irq Returns the IRQ type supporting numIrqsDesire request; ignore
     *        if (return == false)
     * @return true upon DUT support, otherwise false
     */
    static bool VerifyAnySchemeSpecifyNum(uint16_t numIrqsDesire,
        enum nvme_irq_type &irq);
    static void SetAnySchemeSpecifyNum(uint16_t numIrqsDesire);
    static uint16_t GetMaxIRQsSupportedAnyScheme();

private:
};


#endif
