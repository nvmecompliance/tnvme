#ifndef _ALLPCIREGS_r10a_H_
#define _ALLPCIREGS_r10a_H_

#include "test.h"


/**
 * The purpose of this class resides in the constructor
 */
class AllPciRegs_r10a : public Test
{
public:
    AllPciRegs_r10a(int fd);
    virtual ~AllPciRegs_r10a();


protected:
    virtual bool RunCoreTest();


private:
    int ReportOffendingBitPos(ULONGLONG val, ULONGLONG expectedVal);
    bool ValidatePciHdrRegisterROAttribute(PciSpc reg);
    bool ValidatePciCapRegisterROAttribute(PciSpc reg);
    bool ValidateDefaultValues();
    bool ValidateROBitsAfterWriting();
};


#endif
