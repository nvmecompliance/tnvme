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

#ifndef _BACKDOOR_H_
#define _BACKDOOR_H_

#include "tnvme.h"


/**
* This interface to any SQ can be volatile and toxic if you are to use it in
* the manner described and the underlying assumptions within dnvme are
* ignored. Do NOT use this unless you are certain you understand the
* implications, it will crash the kernel, or cause strange behavior.
*
* The framework attempts to allow the maximum flexibility, but should never
* allow crashing the kernel. Thus dnvme implements safe-guards and error
* checking only to those things which could bring about a catastrophe. Certain
* assumptions have been made by dnvme to carry out cmd execution in a general
* fashion, like tracking PRP lists and IRQ’s, etc. If these assumptions are
* violated the kernel will most likely crash.
*
* There are conditions for which we want to test, or inject illegal conditions
* to verify proper rejection by a device. But these attempts violate the
* assumptions made by dnvme. Originally the intention was to do these things
* as a “hybrid approach” where they would be placed into the kernel as canned
* tests cases. However it was realized that the same issues are relevant in the
* kernel and that it would be just as efficient, or more so, to do them in user
* space, so as long as the developer knew the risks.
*
* Therefore, a backdoor was introduced, which could allow send a LEGAL cmd to
* dnvme, conforming to the proper interaction, but before ringing the doorbell,
* one could inject an illegal value into some position into that cmd. The idea
* is that the device will be rejecting the cmd and the fail safe checking could
* be avoided in these rare cases.
*
* This is not the norm. The tnvme/dnvme framework already allowed injecting
* illegal values for a cmd. This interface is ONLY meant to allow illegal
* things that are in most cases probably something a tester would never want
* to do, and are therefore rejected by dnvme outright. These 1-off test
* scenarios can be done but only if the developer truly understand the risks
* being violated.
*
* ******************************************************************************
* If you find yourself using this interface, in most cases your probably not
* going about the test in the correct manner. It is recommended to contact
* nvmecompliance@intel.com to toss out your idea to valid the coding attempt.
* ******************************************************************************
*
* @note This class may throw exceptions.
*/
class Backdoor
{
public:
    /**
     * @param fd Pass the opened file descriptor for the device under test
     */
    Backdoor(int fd);
    virtual ~Backdoor();


    /**
     * See warning at class header. Inject a toxic cmd value to any cmd in any
     * SQ for which its doorbell has NOT YET RUNG.
     * @param injectReq Pass the cmd backdoor injection request
     * @return Nothing, else throws upon errors
     */
    void SetToxicCmdValue(struct backdoor_inject &injectReq);


private:
    Backdoor();

    /// file descriptor to the device under test
    int mFD;
};


#endif
