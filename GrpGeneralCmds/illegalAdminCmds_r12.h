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

#ifndef _ILLEGALADMINCMDS_r12_H_
#define _ILLEGALADMINCMDS_r12_H_

#include <list>
#include "test.h"
#include "illegalAdminCmds_r10b.h"

namespace GrpGeneralCmds {


/** \verbatim
 * -----------------------------------------------------------------------------
 * ----------------Mandatory rules for children to follow-----------------------
 * -----------------------------------------------------------------------------
 * 1) See notes in the header file of the Test base class
 * \endverbatim
 */
class IllegalAdminCmds_r12 : public IllegalAdminCmds_r10b
{
public:
    IllegalAdminCmds_r12(string grpName, string testName);
    virtual ~IllegalAdminCmds_r12();

    /**
     * IMPORTANT: Read Test::Clone() header comment.
     */
    virtual IllegalAdminCmds_r12 *Clone() const
        { return new IllegalAdminCmds_r12(*this); }
    IllegalAdminCmds_r12 &operator=(const IllegalAdminCmds_r12 &other);
    IllegalAdminCmds_r12(const IllegalAdminCmds_r12 &other);


protected:
    /// To allow future revisions of this test to override this functionality
    virtual list<uint8_t> GetIllegalOpcodes();
    static const uint8_t NAMESPACE_MNGMT_OPCODE;
    static const uint8_t NAMESPACE_ATTACH_OPCODE;


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator=().
    ///////////////////////////////////////////////////////////////////////////
};

}   // namespace

#endif
