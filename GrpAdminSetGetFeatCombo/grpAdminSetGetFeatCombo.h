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

#ifndef _GRPADMINSETGETFEATCOMBO_H_
#define _GRPADMINSETGETFEATCOMBO_H_

#include "../group.h"
#include "../Queues/acq.h"
#include "../Queues/asq.h"

#define MAX_IVEC        2048


namespace GrpAdminSetGetFeatCombo {


/**
*  This class implements tests for Admin cmd set/get features utilizing
*  both cmds for efficiency..
*/
class GrpAdminSetGetFeatCombo : public Group
{
public:
    GrpAdminSetGetFeatCombo(size_t grpNum);
    virtual ~GrpAdminSetGetFeatCombo();

protected:
    virtual bool SaveState();
    virtual bool RestoreState();

private:
    uint32_t mArbitration;
    uint32_t mPowerState;
    uint32_t mTmpThreshold;
    uint32_t mTimeLimErrRec;
    uint32_t mVolWrCache;
    uint32_t mIrqCoalescing;
    uint32_t mIvecConf[MAX_IVEC];
    uint32_t mWrAtomicity;
    uint32_t mAsyncEvent;

    void SaveArbitration(SharedASQPtr asq, SharedACQPtr acq);
    bool RestoreArbitration(SharedASQPtr asq, SharedACQPtr acq);

    void SavePowerState(SharedASQPtr asq, SharedACQPtr acq);
    bool RestorePowerState(SharedASQPtr asq, SharedACQPtr acq);

    void SaveTMPTH(SharedASQPtr asq, SharedACQPtr acq);
    bool RestoreTMPTH(SharedASQPtr asq, SharedACQPtr acq);

    void SaveTLER(SharedASQPtr asq, SharedACQPtr acq);
    bool RestoreTLER(SharedASQPtr asq, SharedACQPtr acq);

    void SaveVolWrCache(SharedASQPtr asq, SharedACQPtr acq);
    bool RestoreVolWrCache(SharedASQPtr asq, SharedACQPtr acq);

    void SaveIRQCoalescing(SharedASQPtr asq, SharedACQPtr acq);
    bool RestoreIRQCoalescing(SharedASQPtr asq, SharedACQPtr acq);

    void SaveIvecConf(SharedASQPtr asq, SharedACQPtr acq);
    bool RestoreSaveIvecConf(SharedASQPtr asq, SharedACQPtr acq);

    void SaveWrAtomicity(SharedASQPtr asq, SharedACQPtr acq);
    bool RestoreWrAtomicity(SharedASQPtr asq, SharedACQPtr acq);

    void SaveAsyncEvent(SharedASQPtr asq, SharedACQPtr acq);
    bool RestoreAsyncEvent(SharedASQPtr asq, SharedACQPtr acq);

};

}   // namespace

#endif
