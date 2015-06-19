/*
 * Copyright (c) 2015, UNH-IOL - All Rights Reserved.
 */

#ifndef _INVALIDFIELDINCMD_r11_H_
#define _INVALIDFIELDINCMD_r11_H_

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
class InvalidFieldInCmd_r11 : public InvalidFieldInCmd_r10b
{
public:
    InvalidFieldInCmd_r11(string grpName, string testName);
    virtual ~InvalidFieldInCmd_r11();

    /**
     * IMPORTANT: Read Test::Clone() header comment.
     */
    virtual InvalidFieldInCmd_r11 *Clone() const
        { return new InvalidFieldInCmd_r11(*this); }
    InvalidFieldInCmd_r11 &operator=(const InvalidFieldInCmd_r11 &other);
    InvalidFieldInCmd_r11(const InvalidFieldInCmd_r11 &other);


protected:
    virtual void getInvalidFIDs(vector<uint16_t> invalidFIDs) const;


private:
    ///////////////////////////////////////////////////////////////////////////
    // Adding a member variable? Then edit the copy constructor and operator=().
    ///////////////////////////////////////////////////////////////////////////
};

}   // namespace

#endif
