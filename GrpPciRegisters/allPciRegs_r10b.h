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

#ifndef _ALLPCIREGS_r10b_H_
#define _ALLPCIREGS_r10b_H_

#include "test.h"

namespace GrpPciRegisters {


/** \verbatim
 * -----------------------------------------------------------------------------
 * ----------------Mandatory rules for children to follow-----------------------
 * -----------------------------------------------------------------------------
 * 1) See notes in the header file of the Test base class
 * \endverbatim
 */
class AllPciRegs_r10b : public Test
{
public:
    AllPciRegs_r10b(string grpName, string testName);
    virtual ~AllPciRegs_r10b();

    /**
     * IMPORTANT: Read Test::Clone() header comment.
     */
    virtual AllPciRegs_r10b *Clone() const
        { return new AllPciRegs_r10b(*this); }
    AllPciRegs_r10b &operator=(const AllPciRegs_r10b &other);
    AllPciRegs_r10b(const AllPciRegs_r10b &other);


protected:
    virtual void RunCoreTest();
    virtual RunType RunnableCoreTest(bool preserve);


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator=().
    ///////////////////////////////////////////////////////////////////////////
    /**
     * Validate the specified PCI hdr register RO bits report correct values if
     * and only if they are not vendor specific.
     * @param reg Pass the register to validate
     * @return returns upon success, otherwise false
     */
    bool ValidatePciHdrRegisterROAttribute(PciSpc reg);

    /**
     * Validate the specified capabilities registers' RO bits report correct
     * values if and only if they are not vendor specific.
     * @param reg Pass the register to validate
     * @return returns upon success, otherwise false
     */
    bool ValidatePciCapRegisterROAttribute(PciSpc reg);

    /**
     * Validate all the registers have default values being reported for
     * the RO bits which are not vendor specific.
     * @return returns upon success, otherwise false
     */
    bool ValidateDefaultValues();

    /**
     * Validate all the registers hare RO after attempting to write to them.
     * @return returns upon success, otherwise false
     */
    bool ValidateROBitsAfterWriting();
};

}   // namespace

#endif
