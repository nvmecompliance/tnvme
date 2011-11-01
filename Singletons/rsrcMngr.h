#ifndef _RSRCMNGR_H_
#define _RSRCMNGR_H_

#include <map>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include "tnvme.h"
#include "trackable.h"

using namespace std;

typedef map<string, SharedTrackablePtr>     TrackableMap;


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
class RsrcMngr
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

    /**
     * Allocate a heap object of the 'type' specified and associate that
     * object with the 'lookupName' to aid for future retrieval. This object
     * will have group lifetime in that it will be freed back to the system
     * after the group has complete executing all of its tests.
     * @param type Pass the type of default object to allocate/construct
     * @param lookupName Pass the associated ID of this object
     * @return Pointer to the allocated object, otherwise NullTrackablePtr
     *         upon errors.
     */
    SharedTrackablePtr
    AllocObj(Trackable::ObjType type, string lookupName);

    /**
     * Returns a previously allocated object from AllocObj().
     * @param lookupName Pass the associated ID of the object to return
     * @return Pointer to the allocated object, otherwise NullTrackablePtr
     *         upon errors.
     */
    SharedTrackablePtr
    GetObj(string lookupName);

    /**
     * Free all objects which were allocated for group lifetime.
     */
    void FreeObj();


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

    /// Storehouse for Group:: lifetime objects
    TrackableMap mObjGrpLife;


    /**
     * Perform all the underlying allocation tasks for this class.
     * @param type Pass the type of default object to allocate/construct
     */
    SharedTrackablePtr
    Allocate(Trackable::ObjType type);
};


#endif
