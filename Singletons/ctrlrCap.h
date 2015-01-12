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

#ifndef _CTRLRCAP_H_
#define _CTRLRCAP_H_

#include "tnvme.h"
#include "dnvme.h"
#include "regDefs.h"
#include "subject.h"


/**
* This class is the access to the Controller Capabilities (CAP) register. It
* is true that direct access to the register is allowed/possible via the
* Register singleton via its Read() and Write members, however there are
* features of this class which envelope value added concepts. This class wraps
* the CAP register to aid in providing the high layer logic needed by test case
* integration into the testing framework.
*/
class CtrlrCap
{
public:
    /**
     * Enforce singleton design pattern.
     * @param fd Pass the opened file descriptor for the device under test
     * @param specRev Pass which compliance is needed to target
     * @return NULL upon error, otherwise a pointer to the singleton
     */
    static CtrlrCap *GetInstance(SpecRev specRev);
    static void KillInstance();
    ~CtrlrCap();

    bool ReadRegCAP(uint64_t &regVal);

    bool GetRESVD0(uint8_t &value);

    bool GetMPSMAX(uint8_t &value);

    bool GetMPSMIN(uint8_t &value);

    bool GetRESVD1(uint8_t &value);

    bool GetCSS(uint8_t &value);

    bool GetNVMCSS(bool &value);

    bool GetRESVD2(uint8_t &value);

    bool GetNSSRS(bool &value);

    bool GetDSTRD(uint8_t &value);

    bool GetTO(uint8_t &value);

    bool GetRESVD3(uint8_t &value);

    bool GetAMS(uint8_t &value);

    bool GetCQR(bool &value);

    bool GetMQES(uint16_t &value);

    void prettyPrint(void);

private:
    // Implement singleton design pattern
    CtrlrCap();
    CtrlrCap(SpecRev specRev);
    static bool mInstanceFlag;
    static CtrlrCap *mSingleton;

    /// which spec release is being targeted
    SpecRev mSpecRev;

    /// Current value of controller capabilities register
    uint64_t mRegCAP;

    bool readSuccess;

    bool GetRegValue(uint16_t &value, uint64_t regMask,
            uint8_t bitShift);
};


#endif
