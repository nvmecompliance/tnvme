#ifndef _RSRCMNGR_H_
#define _RSRCMNGR_H_

#include "tnvme.h"
#include "objRsrc.h"
#include "metaRsrc.h"
#include "ctrlrConfig.h"


/**
* This class will automate the creation and deletion of system resources to
* aid in test development. The mundane task of cleaning up utilized resources
* after a single test or group of tests can be made easier. Any object created
* by the RsrcMngr will have group lifetime, meaning its life will be guaranteed
* for the full duration of any single execution group. Test lifetimes will be
* controlled by using shared_ptr's within the Test child class since those
* objects are destroyed completely after they execute, thus any share_ptr will
* also be destroyed and thus so will the objects they point to. Group lifetime
* allows creating objects that will be persistent through all the tests within
* any single execution group.
*
* @note Singleton's are not allowed to throw exceptions.
*/
class RsrcMngr : public ObjRsrc, public MetaRsrc, public ObserverCtrlrState
{
public:
    /**
     * Enforce singleton design pattern.
     * @param fd Pass the opened file descriptor for the device under test
     * @param specRev Pass which compliance is needed to target
     */
    static RsrcMngr* GetInstance(int fd, SpecRev specRev);
    static void KillInstance();
    ~RsrcMngr();

    /// Base class observer pattern requirement, do not call directly.
    void Update(const enum nvme_state &state);


private:
    // Implement singleton design pattern
    RsrcMngr();
    RsrcMngr(int fd, SpecRev specRev);
    static bool mInstanceFlag;
    static RsrcMngr *mSingleton;

    /// which spec release is being targeted
    SpecRev mSpecRev;
    /// file descriptor to the device under test
    int mFd;
};


#endif
