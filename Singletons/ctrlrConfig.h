#ifndef _CTRLRCONFIG_H_
#define _CTRLRCONFIG_H_

#include "../tnvme.h"


/**
* This class is the access to the Controller Configuration (CC) register. It
* is true that direct access to the register is allowed/possible via the
* Register singleton via its Read() and Write members, however there are
* features of this class which envelope value added concepts. This class wraps
* the CC register to aid in providing the high layer logic needed by test case
* integration into the testing framework.
*
* @note Singleton's are not allowed to throw exceptions.
*/
class CtrlrConfig
{
public:
    /**
     * Enforce singleton design pattern.
     * @param fd Pass the opened file descriptor for the device under test
     * @param specRev Pass which compliance is needed to target
     */
    static CtrlrConfig* GetInstance(int fd, SpecRev specRev);
    static void KillInstance();
    ~CtrlrConfig();


private:
    // Implement singleton design pattern
    CtrlrConfig();
    CtrlrConfig(int fd, SpecRev specRev);
    static bool mInstanceFlag;
    static CtrlrConfig *mSingleton;

    /// which spec release is being targeted
    SpecRev mSpecRev;
    /// file descriptor to the device under test
    int mFd;
};


#endif
