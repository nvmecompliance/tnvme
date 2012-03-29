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

    /*
     * Does the DUT support discontiguous IOQ's.
     * @return true if DUT supports discontig backed IOQ memory, otherwise false
     */
    static bool SupportDiscontigIOQ();

    /**
     * Creates, in hdw, an IOCQ and returns the resource.
     * This method requires 0 elements to reside in the CQ and also assume no
     * other cmd will complete into that CQ while this operation is occurring.
     * In the end the number of IRQ's and number of CE's will be verified to
     * guarantee that only 1 CE arrived as a result of sending this 1 cmd.
     * @note Method uses pre-existing values of CC.IOCQES
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
     * @param qualify Pass a qualifying string to append to each dump file
     * @param verbose Pass true to dump resources to dump files, otherwise false
     * @return The newly create object or throws upon errors
     */
    static SharedIOCQPtr CreateIOCQContigToHdw(string grpName,
        string testName, uint16_t ms, SharedASQPtr asq, SharedACQPtr acq,
        uint16_t qId, uint32_t numEntries, bool grpLifetime, string grpID,
        bool irqEnabled, uint16_t irqVec, string qualify = "",
        bool verbose = true);
    /// param qBackedMem is not modified, nor err chk'd; it must setup by caller
    static SharedIOCQPtr CreateIOCQDiscontigToHdw(string grpName,
        string testName, uint16_t ms, SharedASQPtr asq, SharedACQPtr acq,
        uint16_t qId, uint32_t numEntries, bool grpLifetime, string grpID,
        bool irqEnabled, uint16_t irqVec, SharedMemBufferPtr qBackedMem,
        string qualify = "", bool verbose = true);

    /**
     * Creates, in hdw, an IOCQ and returns the resource.
     * This method requires 0 elements to reside in the CQ and also assume no
     * other cmd will complete into that CQ while this operation is occurring.
     * In the end the number of IRQ's and number of CE's will be verified to
     * guarantee that only 1 CE arrived as a result of sending this 1 cmd.
     * @note Method uses pre-existing values of CC.IOSQES
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
     * @param qualify Pass a qualifying string to append to each dump file
     * @param verbose Pass true to dump resources to dump files, otherwise false
     * @return The newly create object or throws upon errors
     */
    static SharedIOSQPtr CreateIOSQContigToHdw(string grpName,
        string testName, uint16_t ms, SharedASQPtr asq, SharedACQPtr acq,
        uint16_t qId, uint32_t numEntries, bool grpLifetime, string grpID,
        uint16_t cqId, uint8_t priority, string qualify = "",
        bool verbose = true);
    /// param qBackedMem is not modified, nor err chk'd; it must setup by caller
    static SharedIOSQPtr CreateIOSQDiscontigToHdw(string grpName,
        string testName, uint16_t ms, SharedASQPtr asq, SharedACQPtr acq,
        uint16_t qId, uint32_t numEntries, bool grpLifetime, string grpID,
        uint16_t cqId, uint8_t priority, SharedMemBufferPtr qBackedMem,
        string qualify = "", bool verbose = true);

    /**
     * Deletes, in hdw, an pre-existing IOCQ.
     * This method requires 0 elements to reside in the CQ and also assume no
     * other cmd will complete into that CQ while this operation is occurring.
     * In the end the number of IRQ's and number of CE's will be verified to
     * guarantee that only 1 CE arrived as a result of sending this 1 cmd.
     * @note Throws upon errors
     * @param grpName Pass the name of the group to which this test belongs
     * @param testName Pass the name of the child testclass
     * @param ms Pass the max number of ms to wait until numTil CE's arrive.
     * @param iocq Pass the object which represents the IOCQ to deleted
     * @param asq Pass pre-existing ASQ to issue a cmd into
     * @param acq Pass pre-existing ACQ to reap CE to verify successful creation
     * @param qualify Pass a qualifying string to append to each dump file
     * @param verbose Pass true to dump resources to dump files, otherwise false
     */
    static void DeleteIOCQToHdw(string grpName, string testName,
        uint16_t ms, SharedIOCQPtr iocq, SharedASQPtr asq, SharedACQPtr acq,
        string qualify = "", bool verbose = true);
    static void DeleteIOSQToHdw(string grpName, string testName,
        uint16_t ms, SharedIOSQPtr iosq, SharedASQPtr asq, SharedACQPtr acq,
        string qualify = "", bool verbose = true);


private:
};


#endif
