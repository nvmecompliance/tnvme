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

#ifndef _CTRLRCONFIG_H_
#define _CTRLRCONFIG_H_

#include "tnvme.h"
#include "dnvme.h"
#include "regDefs.h"
#include "subject.h"

/// Subject/Observer pattern for SetState() actions within CtrlrConfig
typedef StateObserver<enum nvme_state> ObserverCtrlrState;
typedef StateSubject<enum nvme_state>  SubjectCtrlrState;


/**
* This class is the access to the Controller Configuration (CC) register. It
* is true that direct access to the register is allowed/possible via the
* Register singleton via its Read() and Write members, however there are
* features of this class which envelope value added concepts. This class wraps
* the CC register to aid in providing the high layer logic needed by test case
* integration into the testing framework.
*/
class CtrlrConfig : public SubjectCtrlrState
{
public:
    /**
     * Enforce singleton design pattern.
     * @param fd Pass the opened file descriptor for the device under test
     * @param specRev Pass which compliance is needed to target
     * @return NULL upon error, otherwise a pointer to the singleton
     */
    static CtrlrConfig *GetInstance(int fd, SpecRev specRev);
    static void KillInstance();
    ~CtrlrConfig();

    static const uint16_t MAX_MSI_SINGLE_IRQ_VEC;
    static const uint16_t MAX_MSI_MULTI_IRQ_VEC;
    static const uint16_t MAX_MSIX_IRQ_VEC;

    /**
     * Gets the active IRQ scheme enabled in the device. It doesn't
     * indicate that IRQ's are being used, to use IRQ's CQ's must be created
     * to use IRQ's. The ACQ doesn't have a choice and always uses IRQ's if
     * there is a ACQ allocated..
     * @param irq Returns wich IRQ scheme is active
     * @param numIrqs Returns the number of IRQ's which are active
     * @return true upon success, otherwise ignore parameter irq.
     */
    bool GetIrqScheme(enum nvme_irq_type &irq, uint16_t &numIrqs);

    /**
     * Set the active IRQ scheme in the device. This only sets up the controller
     * to use this IRQ scheme, in order to use the scheme CQ's must be created.
     * The controller must be disable for this call to succeed.
     * @param newIrq Pass the new IRQ scheme which the controller should use.
     * @param numIrqs Pass the total number of IRQ's to request
     * @return true upon success, otherwise false.
     */
    bool SetIrqScheme(enum nvme_irq_type newIrq, uint16_t numIrqs);

    /**
     * Determines whether IRQ's are:
     * disabled (nvme_irq_type == {INT_NONE | INT_FENCE}), or
     * enabled (nvme_irq_type == {INT_MSI_SINGLE | INT_MSI_MULTI | INT_MSIX}).
     * @return true if enabled, otherwise false
     */
    bool IrqsEnabled();

    /**
     * Learns of the spec'd IRQ capability by interrogating the DUT's Registers.
     * @param capable returns true if IRQ scheme supported, otherwise false
     * @param numIrqs Returns the number of IRQ's supported under the scheme.
     *        if (capable == false) then ignore this parameter.
     * @return true upon successful DUT interrogation, otherwise false
     */
    bool IsMSICapable(bool &capable, uint16_t &numIrqs);
    bool IsMSIXCapable(bool &capable, uint16_t &numIrqs);

    /**
     * Is the controller enabled?
     * @return true if enabled, otherwise false
     */
    bool IsStateEnabled();

    /**
     * Set the state of the controller.
     * @param state Pass {ST_ENABLE | ST_DISABLE | ST_DISABLE_COMPLETELY}
     *      ST_ENABLE to enable the controller;
     *      ST_DISABLE to disable, free all kernel memory except what is
     *          needed by the ACQ/ASQ, because those entities remain intact.
     *          The ACQ/ASQ are also reset to the empty state. A re-enabling at
     *          this point would allow the immediate submission of admin cmds
     *          into ACQ. The action causes dnvme to automatically
     *          invoke SetIrqScheme(INT_NONE).
     *      ST_DISABLE_COMPLETELY to disable and nothing is left intact. This
     *          is as close to a power up situation as one could achieve. The
     *          NVME device resets all registers to default values and dnvme
     *          writes admin Q base addresses and Q sizes to 0, nothing is truly
     *          enabled. The action causes dnvme to automatically invoke
     *          SetIrqScheme(INT_NONE).
     * @return true if successful, otherwise false
     */
    bool SetState(enum nvme_state state);

    bool ReadRegCC(uint32_t &regVal);
    bool WriteRegCC(uint32_t regVal);

    bool GetIOCQES(uint8_t &value);
    bool SetIOCQES(uint8_t value);

    bool GetIOSQES(uint8_t &value);
    bool SetIOSQES(uint8_t value);

    bool GetSHN(uint8_t &value);
    bool SetSHN(uint8_t value);

    bool GetAMS(uint8_t &value);
    bool SetAMS(uint8_t value);

    bool GetMPS(uint8_t &value);

    bool GetCSS(uint8_t &value);
    bool SetCSS(uint8_t value);
    static const uint8_t CSS_NVM_CMDSET;


private:
    // Implement singleton design pattern
    CtrlrConfig();
    CtrlrConfig(int fd, SpecRev specRev);
    static bool mInstanceFlag;
    static CtrlrConfig *mSingleton;

    /// which spec release is being targeted
    SpecRev mSpecRev;
    /// file descriptor to the device under test
    int mFd;

    /// Current value of controller capabilities register
    uint32_t mRegCAP;

    bool GetRegValue(uint8_t &value, uint32_t regMask, uint8_t bitShift);
    bool SetRegValue(uint8_t value, uint8_t valueMask, uint64_t regMask,
        uint8_t bitShift);

    /// Converts an enum to a human readable string
    bool DecodeIrqScheme(enum nvme_irq_type newIrq, string &desc);

    /// Set page size according to what sysconf(_SC_PAGESIZE) returns
    bool SetMPS();
};


#endif
