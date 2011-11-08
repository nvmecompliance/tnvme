#ifndef _OBJRSRC_H_
#define _OBJRSRC_H_

#include <map>
#include <boost/shared_ptr.hpp>
#include "trackable.h"
#include "tnvme.h"


/**
* This base class will handle object resources.
*
* @note @note This class does not throw exceptions.
*/
class ObjRsrc
{
public:
    /**
     * @param fd Pass the opened file descriptor for the device under test
     */
    ObjRsrc(int fd);
    ~ObjRsrc();

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

    /// Free all objects which were allocated for group lifetime.
    void FreeAllObj();


private:
    // Implement singleton design pattern
    ObjRsrc();

    /// file descriptor to the device under test
    int mFd;

    /// Storehouse for Group:: lifetime objects
    typedef map<string, SharedTrackablePtr> TrackableMap;
    TrackableMap mObjGrpLife;

    /**
     * Perform all the underlying allocation tasks for this class.
     * @param type Pass the type of default object to allocate/construct
     */
    SharedTrackablePtr
    Allocate(Trackable::ObjType type);
};


#endif
