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

#ifndef _QUEUES_H_
#define _QUEUES_H_

#include "tnvme.h"
#include "dnvme.h"
#include "../Queues/asq.h"
#include "../Queues/acq.h"
#include "../Cmds/createIOCQ.h"
#include "../Cmds/createIOSQ.h"


/**
* This class is meant not be instantiated because it should only ever contain
* static members. These utility functions can be viewed as wrappers to
* perform common, repetitious tasks which avoids coping the same chunks of
* code throughout the framework.
*
* @note This class may throw exceptions, please see comment within specific
*       methods.
*/
class Queues
{
public:
    Queues();
    virtual ~Queues();

    /**
     * Creates, in hdw, an IOCQ and returns the resource.
     * @note Method uses pre-existing values of CC.IOCQES
     * @param fd Pass the device FD, not the filename, to communicate
     * @param grpName Pass the name of the group to which this test belongs
     * @param testName Pass the name of the child testclass
     * @param ms Pass the max number of ms to wait until numTil CE's arrive.
     * @param asq Pass pre-existing ASQ to issue a cmd into
     * @param acq Pass pre-existing ACQ to reap CE to verify successful creation
     * @param qId Pass the queue's ID
     * @param numEntries Pass the number of elements within the Q
     * @param grpLifetime Pass true to create with group lifetime, false for
     *        test lifetime
     * @param grpID Pass the IOSQ's group lifetime ID to become assoc'd with;
     *        Value is ignore if (grpLifetime == false).
     * @param irqEnabled Pass true if IRQ's are to be enabled for this Q
     * @param irqVec If (irqEnabled==true) then spec's the IRQ's vector. Value
     *        must be among the set from (0 - (n-1)) where n is is spec'd
     *        during a call to CtrlrConfig::SetIrqScheme().
     * @return The newly create object or throws upon errors
     */
    static SharedCreateIOCQPtr CreateIOCQContig(int fd, string grpName,
        string testName, uint16_t ms, SharedASQPtr asq, SharedACQPtr acq,
        uint16_t qId, uint16_t numEntries, bool grpLifetime, string grpID,
        bool irqEnabled, uint16_t irqVec);
    /// param qBackedMem is not modified, nor err chk'd; it must setup by caller
    static SharedCreateIOCQPtr CreateIOCQDiscontig(int fd, string grpName,
        string testName, uint16_t ms, SharedASQPtr asq, SharedACQPtr acq,
        uint16_t qId, uint16_t numEntries, bool grpLifetime, string grpID,
        bool irqEnabled, uint16_t irqVec,
        SharedMemBufferPtr qBackedMem);

    /**
     * Creates, in hdw, an IOCQ and returns the resource.
     * @note Method uses pre-existing values of CC.IOSQES
     * @param fd Pass the device FD, not the filename, to communicate
     * @param grpName Pass the name of the group to which this test belongs
     * @param testName Pass the name of the child testclass
     * @param ms Pass the max number of ms to wait until numTil CE's arrive.
     * @param asq Pass pre-existing ASQ to issue a cmd into
     * @param acq Pass pre-existing ACQ to reap CE to verify successful creation
     * @param qId Pass the queue's ID
     * @param numEntries Pass the number of elements within the Q
     * @param grpLifetime Pass true to create with group lifetime, false for
     *        test lifetime
     * @param grpID Pass the IOSQ's group lifetime ID to become assoc'd with;
     *        Value is ignore if (grpLifetime == false).
     * @param cqId Pass the assoc CQ ID to which this SQ will be associated
     * @param priority Pass this Q's priority value, must be a 2 bit value
     * @return The newly create object or throws upon errors
     */
    static SharedCreateIOSQPtr CreateIOSQContig(int fd, string grpName,
        string testName, uint16_t ms, SharedASQPtr asq, SharedACQPtr acq,
        uint16_t qId, uint16_t numEntries, bool grpLifetime, string grpID,
        uint16_t cqId, uint8_t priority);
    /// param qBackedMem is not modified, nor err chk'd; it must setup by caller
    static SharedCreateIOSQPtr CreateIOSQDiscontig(int fd, string grpName,
        string testName, uint16_t ms, SharedASQPtr asq, SharedACQPtr acq,
        uint16_t qId, uint16_t numEntries, bool grpLifetime, string grpID,
        uint16_t cqId, uint8_t priority,
        SharedMemBufferPtr qBackedMem);

    /**
     * Deletes, in hdw, an pre-existing IOCQ.
     * @note Throws upon errors
     * @param fd Pass the device FD, not the filename, to communicate
     * @param grpName Pass the name of the group to which this test belongs
     * @param testName Pass the name of the child testclass
     * @param ms Pass the max number of ms to wait until numTil CE's arrive.
     * @param iocq Pass the object which represents the IOCQ to deleted
     * @param asq Pass pre-existing ASQ to issue a cmd into
     * @param acq Pass pre-existing ACQ to reap CE to verify successful creation
     */
    static void DeleteIOCQInHdw(int fd, string grpName, string testName,
        uint16_t ms, SharedIOCQPtr iocq, SharedASQPtr asq, SharedACQPtr acq);
    static void DeleteIOSQInHdw(int fd, string grpName, string testName,
        uint16_t ms, SharedIOSQPtr iosq, SharedASQPtr asq, SharedACQPtr acq);


private:
    static SharedCreateIOCQPtr InvokeIOCQInHdw(int fd, string grpName,
        string testName, uint16_t ms, SharedASQPtr asq, SharedACQPtr acq,
        SharedIOCQPtr iocq, string qualify);
    static SharedCreateIOSQPtr InvokeIOSQInHdw(int fd, string grpName,
        string testName, uint16_t ms, SharedASQPtr asq, SharedACQPtr acq,
        SharedIOSQPtr iosq, string qualify);
    static void ReapCE(SharedACQPtr acq, uint16_t numCE);
};


#endif
