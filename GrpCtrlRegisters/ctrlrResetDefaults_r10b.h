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

#ifndef _CTRLRRESETDEFAULTS_r10b_H_
#define _CTRLRRESETDEFAULTS_r10b_H_

#include <map>
#include "test.h"

namespace GrpCtrlRegisters {


/** \verbatim
 * -----------------------------------------------------------------------------
 * ----------------Mandatory rules for children to follow-----------------------
 * -----------------------------------------------------------------------------
 * 1) See notes in the header file of the Test base class
 * \endverbatim
 */
class CtrlrResetDefaults_r10b : public Test
{
public:
    CtrlrResetDefaults_r10b(string grpName, string testName);
    virtual ~CtrlrResetDefaults_r10b();

    /**
     * IMPORTANT: Read Test::Clone() header comment.
     */
    virtual CtrlrResetDefaults_r10b *Clone() const
        { return new CtrlrResetDefaults_r10b(*this); }
    CtrlrResetDefaults_r10b &operator=(const CtrlrResetDefaults_r10b &other);
    CtrlrResetDefaults_r10b(const CtrlrResetDefaults_r10b &other);


protected:
    virtual void RunCoreTest();
    virtual RunType RunnableCoreTest(bool preserve);


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator=().
    ///////////////////////////////////////////////////////////////////////////

    /**
     * Verify all the approp cntrl'r space registers are reset to default
     * values after a cntrl'r reset is issued.
     * @return returns upon success, otherwise throws exception
     */
    void VerifyCtrlrResetDefaults();

    /**
     * Modify RW Bits of the ctrl'r space registers except AQA, ASQ
     * and ACQ because ctrl'r reset should not changes these registers.
     * @param ctrlRegsMap Pass the regs to be remembered: AQA, ASQ and ACQ
     * @return returns upon success, otherwise throws exception
     */
    void ModifyRWBitsOfCtrlrRegisters(std::map <int, uint64_t>& ctrlRegsMap);

    /**
     * Validate the specified ctrl'r register RW bits report correct default
     * values after a ctrl'r reset is issued.
     * @param ctrlRegsMap Pass the register values for AQA, ASQ and ACQ
     * @return returns upon success, otherwise throws exception
     */
    void ValidateCtrlrRWDefaultsAfterReset(std::map <int, uint64_t>&
        ctrlRegsMap);
};

}   // namespace

#endif
