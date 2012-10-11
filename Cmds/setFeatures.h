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

#ifndef _SETFEATURES_H_
#define _SETFEATURES_H_

#include "baseFeatures.h"


class SetFeatures;    // forward definition
typedef boost::shared_ptr<SetFeatures>              SharedSetFeaturesPtr;
typedef boost::shared_ptr<const SetFeatures>        ConstSharedSetFeaturesPtr;
#define CAST_TO_SETFEATURES(shared_trackable_ptr)   \
        boost::shared_polymorphic_downcast<SetFeatures>(shared_trackable_ptr);


/**
* This class implements the SetFeatures admin cmd
*
* @note This class may throw exceptions.
*/
class SetFeatures : public BaseFeatures
{
public:
    SetFeatures();
    virtual ~SetFeatures();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedSetFeaturesPtr NullSetFeaturesPtr;
    static const uint8_t Opcode;

    /**
     * Set the command arbitration desired.
     * @param hpw Pass the high priority weight desired
     * @param mpw Pass the medium priority weight desired
     * @param lpw Pass the low priority weight desired
     * @param ab Pass the arbitration burst desired
     */
    void SetArbitration(uint8_t hpw, uint8_t mpw, uint8_t lpw, uint8_t ab);
    void SetArbitrationHPW(uint8_t hpw);
    void SetArbitrationMPW(uint8_t mpw);
    void SetArbitrationLPW(uint8_t lpw);
    void SetArbitrationAB(uint8_t ab);
    uint32_t GetArbitration() const;
    uint8_t GetArbitrationHPW() const;
    uint8_t GetArbitrationMPW() const;
    uint8_t GetArbitrationLPW() const;
    uint8_t GetArbitrationAB() const;

    /**
     * Set the power management state (PS) desired.
     * @note Spec states "shall only" be set to supported values using
     *       IdentifyCtrlr.NPSS
     * @param ps Pass the power state desired
     */
    void SetPowerManagementPS(uint8_t ps);
    uint8_t GetPowerManagementPS() const;

    /**
     * Set the number of LBA ranges desired.
     * @param num Pass the number of LBA ranges desired
     */
    void SetLBARangeTypeNUM(uint8_t num);
    uint8_t GetLBARangeTypeNUM() const;

    /**
     * Set the temperature threshold desired in units of kelvin.
     * @param tmpth Pass the temperature threshold desired (kelvin)
     */
    void SetTempThresholdTMPTH(uint16_t tmpth);
    uint16_t GetTempThresholdTMPTH() const;

    /**
     * Set the error recovery limited retry timeout desired.
     * This value is in 100 millisec units.
     * @param tler Pass the time limited error recovery desired (100ms units)
     */
    void SetErrRecoveryTLER(uint16_t tler);
    uint16_t GetErrRecoveryTLER() const;

    /**
     * Set the volatile write cache to either enabled or disabled.
     * @param wce Pass write cache enabled(1) or disabled(0).
     */
    void SetVolatileWriteCacheWCE(uint8_t wce);
    uint8_t GetVolatileWriteCacheWCE() const;

    /**
     * Set the number of IO queues desired.
     * @note Spec states  "shall only" be set after a power cycle, not resets
     * @param ncqr Pass the number of IOCQ's desired
     * @param nsqr Pass the number of IOSQ's desired
     */
    void SetNumberOfQueues(uint16_t ncqr, uint16_t nsqr);
    void SetNumberOfQueuesNCQR(uint16_t ncqr);
    void SetNumberOfQueuesNSQR(uint16_t nsqr);
    uint32_t GetNumberOfQueues() const;
    uint16_t GetNumberOfQueuesNCQR() const;
    uint16_t GetNumberOfQueuesNSQR() const;

    /**
     * Set the interrupt coalescing values for aggregation time in 100us
     * increments and aggregation threshold (completion entries) desired.
     * @param time Pass recommended maximum time in 100 usec units.
     * @param thr Pass desired minimum no. of CQ entries to aggregate per IV.
     */
    void SetIntCoalescing(uint8_t time, uint8_t thr);
    void SetIntCoalescingTIME(uint8_t time);
    void SetIntCoalescingTHR(uint8_t thr);
    uint32_t GetIntCoalescing() const;
    uint8_t GetIntCoalescingTIME() const;
    uint8_t GetIntCoalescingTHR() const;

    /**
     * Set the interrupt vector configuration desired.
     * @param cd Set coalescing disable value to 1 or 0 desired.
     * @param iv Specify the interrupt vec for which the config settings apply.
     */
    void SetIntVecConfig(uint8_t cd, uint16_t iv);
    void SetIntVecConfigIV(uint16_t iv);
    void SetIntVecConfigCD(uint8_t cd);
    uint32_t GetIntVecConfig() const;
    uint16_t GetIntVecConfigIV() const;
    uint8_t GetIntVecConfigCD() const;

    /**
     * Set the write atomicity feature control desired
     * @param dn Set the write atomicity of enabled(1) or disable(0).
     */
    void SetWriteAtomicityDN(uint8_t dn);
    uint8_t GetWriteAtomicityDN() const;

    /**
     * Set the desired controls in bit vector for the events that trigger an
     * asynchronous event notification to the host.
     * @param critWarn Set the desired bit mask for critical warnings.
     */
    void SetAsyncEventConfigSMART(uint16_t critWarn);
    uint16_t GetAsyncEventConfigSMART() const;

    /**
     * Set the pre boot s/w load cnt desired for software progress marker
     * and this is persistent across power cycles.
     * @param pbslc Pass pre-boot s/w load cnt desired.
     */
    void SetSWProgressMarkerPBSLC(uint8_t pbslc);
    uint8_t GetSWProgressMarkerPBSLC() const;
};


#endif
