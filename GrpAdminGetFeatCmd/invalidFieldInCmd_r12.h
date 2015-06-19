/*
 * Copyright (c) 2015, UNH-IOL - All Rights Reserved.
 */

#ifndef _INVALIDFIELDINCMD_r12_H_
#define _INVALIDFIELDINCMD_r12_H_

#include "test.h"
#include "invalidFieldInCmd_r10b.h"

namespace GrpAdminGetFeatCmd {


/** \verbatim
 * -----------------------------------------------------------------------------
 * ----------------Mandatory rules for children to follow-----------------------
 * -----------------------------------------------------------------------------
 * 1) See notes in the header file of the Test base class
 * \endverbatim
 */
class InvalidFieldInCmd_r12 : public InvalidFieldInCmd_r10b
{
public:
    InvalidFieldInCmd_r12(string grpName, string testName);
    virtual ~InvalidFieldInCmd_r12();

    /**
     * IMPORTANT: Read Test::Clone() header comment.
     */
    virtual InvalidFieldInCmd_r12 *Clone() const
        { return new InvalidFieldInCmd_r12(*this); }
    InvalidFieldInCmd_r12 &operator=(const InvalidFieldInCmd_r12 &other);
    InvalidFieldInCmd_r12(const InvalidFieldInCmd_r12 &other);


protected:
    virtual void getInvalidFIDs(vector<uint16_t> invalidFIDs) const;


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator=().
    ///////////////////////////////////////////////////////////////////////////
};

}   // namespace

#endif
