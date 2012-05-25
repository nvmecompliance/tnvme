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

#ifndef _OBJRSRC_H_
#define _OBJRSRC_H_

#include <map>
#include <boost/shared_ptr.hpp>
#include "trackable.h"
#include "tnvme.h"


/**
* This base class will handle object resources.
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


protected:
    /// Free all objects which were allocated.
    void FreeAllObj();

    /// Free all objects which were allocated, except ACQ/ASQ
    void FreeAllObjNotASQACQ();


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
    AllocWorker(Trackable::ObjType type);
};


#endif
