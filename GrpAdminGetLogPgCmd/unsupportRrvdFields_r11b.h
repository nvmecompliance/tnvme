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

#ifndef _UNSUPPORTRRVDFIELDS_r11b_H_
#define _UNSUPPORTRRVDFIELDS_r11b_H_

#include "test.h"

namespace GrpAdminGetLogPgCmd {


/** \verbatim
 * -----------------------------------------------------------------------------
 * ----------------Mandatory rules for children to follow-----------------------
 * -----------------------------------------------------------------------------
 * 1) See notes in the header file of the Test base class
 * \endverbatim
 */
class UnsupportRrvdFields_r11b : public Test
{
public:
    UnsupportRrvdFields_r11b(string grpName, string testName);
    virtual ~UnsupportRrvdFields_r11b();

    /**
     * IMPORTANT: Read Test::Clone() header comment.
     */
    virtual UnsupportRrvdFields_r11b *Clone() const
        { return new UnsupportRrvdFields_r11b(*this); }
    UnsupportRrvdFields_r11b &operator=(const UnsupportRrvdFields_r11b &other);
    UnsupportRrvdFields_r11b(const UnsupportRrvdFields_r11b &other);


protected:
    virtual void RunCoreTest();
    virtual RunType RunnableCoreTest(bool preserve);;


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator=().
    ///////////////////////////////////////////////////////////////////////////
};

}   // namespace

#endif
