#ifndef _RSRCMNGR_H_
#define _RSRCMNGR_H_

#include <map>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include "trackable.h"
#include "../tnvme.h"

using namespace std;

typedef boost::shared_ptr<Trackable>        SharedTrackablePtr;
typedef map<string, SharedTrackablePtr>     TrackableMap;
typedef vector<SharedTrackablePtr>          TrackableVector;



/**
* This class will automate the creation and deletion of system resources to
* aid in test development. The mundane task of cleaning up utilized resources
* after a single test or group of tests has run is made easier. This object
* will destroy resources that it creates automatically when their lifetimes
* have expired.
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

    /// Used to compare for NULL pointers being returned by allocations
    static SharedTrackablePtr NullTrackablePtr;

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
    AllocObjGrpLife(Trackable::ObjType type, string lookupName);

    /**
     * Allocate a heap object of the 'type' specified. This object will have
     * test lifetime in that it will be freed back to the system after the
     * test which performs the allocation completes.
     * @param type Pass the type of default object to allocate/construct
     * @return Pointer to the allocated object, otherwise NullTrackablePtr
     *         upon errors.
     */
    SharedTrackablePtr
    AllocObjTestLife(Trackable::ObjType type);

    /**
     * Returns a previously allocated object from AllocObjGrpLife().
     * @param lookupName Pass the associated ID of the object to return
     * @return Pointer to the allocated object, otherwise NullTrackablePtr
     *         upon errors.
     */
    SharedTrackablePtr
    GetObjGrpLife(string lookupName);

    /**
     * Free all objects which were allocated for group lifetime. This method
     * automatically calls FreeObjTestLife() for test lifetimes are shorter
     * than group lifetimes.
     */
    void FreeObjGrpLife();

    /**
     * Free all objects which were allocated for test lifetime.
     */
    void FreeObjTestLife();


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
    /// Storehouse for Test:: lifetime objects
    TrackableVector mObjTesLife;


    /**
     * Perform all the underlying allocation tasks for this class.
     * @param type Pass the type of default object to allocate/construct
     * @param testLifeTime Pass true to test lifetime, false for group lifetime
     */
    SharedTrackablePtr
    AllocObj(Trackable::ObjType type, bool testLifeTime);
};


#endif
