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
     * Set the number of IO queues desired.
     * @note Spec states  "shall only" be set after a power cycle, not resets
     * @param ncqr Pass the number of IOCQ's desired
     * @param nsqr Pass the number of IOSQ's desired
     */
    void SetNumberOfQueues(uint16_t ncqr, uint16_t nsqr);
    uint32_t GetNumberOfQueues() const;

    /**
     * Set the command arbitration desired.
     * @param arb Pass the arbitration desired
     */
    void SetArbitration(uint32_t arb);
    uint32_t GetArbitration() const;

    /**
     * Set the power state descriptor desired.
     * @note Spec states "shall only" be set to supported values using
     *       IdentifyCtrlr.NPSS
     * @param psd Pass the power state descriptor desired
     */
    void SetPSD(uint8_t psd);
    uint8_t GetPSD() const;

    /**
     * Set the temperature threshold desired in units of kelvin.
     * @param tmpth Pass the temperature threshold desired (kelvin)
     */
    void SetTempThreshold(uint16_t tmpth);
    uint16_t GetTempThreshold() const;

    /**
     * Set the error recovery limited retry timeout desired.
     * This value is in 100 millisec units.
     * @param tler Pass the time limited error recovery desired (100ms units)
     */
    void SetErrRecoveryTime(uint16_t tler);
    uint16_t GetErrRecoveryTime() const;

    /**
     * Set the volatile write cache to either enabled or disabled.
     * @param wce Pass write cache enabled(1) or disabled(0).
     */
    void SetVolatileWriteCache(uint8_t wce);
    uint8_t GetVolatileWriteCache() const;

    /**
     * Set the interrupt coalescing values for aggregation time in 100us
     * increments and aggregation threshold (completion entries) desired.
     * @param aTime Pass recommended maximum time in 100 usec units.
     * @param aThr Pass desired minimum no. of CQ entries to aggregate per IV.
     */
    void SetIntCoalescing(uint8_t aTime, uint8_t aThr);
    uint16_t GetIntCoalescing() const;

    /**
     * Set the interrupt vector configuration desired.
     * @param cd Set coalescing disable value to 1 or 0 desired.
     * @param iv Specify the interrupt vec for which the config settings apply.
     */
    void SetIntVecConfig(uint8_t cd, uint16_t iv);
    uint32_t GetIntVecConfig() const;

    /**
     * Set the desired controls in bit vector for the events that trigger an
     * asynchronous event notification to the host.
     * @param critWarn Set the desired bit mask for critical warnings.
     */
    void SetAsyncEventConfig(uint16_t critWarn);
    uint16_t GetAsyncEventConfig() const;

    /**
     * Set the pre boot s/w load cnt desired for software progress marker
     * and this is persistent across power cycles.
     * @param pbslc Pass pre-boot s/w load cnt desired.
     */
    void SetSWProgressMarker(uint8_t pbslc);
    uint8_t GetSWProgressMarker() const;
};


#endif
