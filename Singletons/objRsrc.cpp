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

#include "objRsrc.h"
#include "memBuffer.h"
#include "../Queues/acq.h"
#include "../Queues/asq.h"
#include "../Queues/iocq.h"
#include "../Queues/iosq.h"
#include "../Cmds/identify.h"
#include "../Cmds/createIOCQ.h"
#include "../Cmds/createIOSQ.h"
#include "../Cmds/deleteIOCQ.h"
#include "../Cmds/deleteIOSQ.h"


typedef pair<string, SharedTrackablePtr> TrackablePair;


ObjRsrc::ObjRsrc()
{
    mFd = 0;
}


ObjRsrc::ObjRsrc(int fd)
{
    mFd = fd;
    if (mFd < 0) {
        LOG_DBG("Object created with a bad FD=%d", fd);
        return;
    }
}


ObjRsrc::~ObjRsrc()
{
}


SharedTrackablePtr
ObjRsrc::AllocWorker(Trackable::ObjType type)
{
    switch (type) {
    case Trackable::OBJ_MEMBUFFER:
        LOG_NRM("MemBuffer is born with group lifetime");
        return SharedTrackablePtr(new MemBuffer());
        break;
    case Trackable::OBJ_ACQ:
        LOG_NRM("ACQ is born with group lifetime");
        return SharedTrackablePtr(new ACQ(mFd));
        break;
    case Trackable::OBJ_ASQ:
        LOG_NRM("ASQ is born with group lifetime");
        return SharedTrackablePtr(new ASQ(mFd));
        break;
    case Trackable::OBJ_IOCQ:
        LOG_NRM("IOCQ is born with group lifetime");
        return SharedTrackablePtr(new IOCQ(mFd));
        break;
    case Trackable::OBJ_IOSQ:
        LOG_NRM("IOSQ is born with group lifetime");
        return SharedTrackablePtr(new IOSQ(mFd));
        break;
    case Trackable::OBJ_IDENTIFY:
        LOG_NRM("Cmd Identify is born with group lifetime");
        return SharedTrackablePtr(new Identify(mFd));
        break;
    case Trackable::OBJ_CREATEIOCQ:
        LOG_NRM("Cmd Create IOCQ is born with group lifetime");
        return SharedTrackablePtr(new CreateIOCQ(mFd));
        break;
    case Trackable::OBJ_CREATEIOSQ:
        LOG_NRM("Cmd Create IOSQ is born with group lifetime");
        return SharedTrackablePtr(new CreateIOSQ(mFd));
        break;
    case Trackable::OBJ_DELETEIOCQ:
        LOG_NRM("Cmd Delete IOCQ is born with group lifetime");
        return SharedTrackablePtr(new DeleteIOCQ(mFd));
        break;
    case Trackable::OBJ_DELETEIOSQ:
        LOG_NRM("Cmd Delete IOSQ is born with group lifetime");
        return SharedTrackablePtr(new DeleteIOSQ(mFd));
        break;
    default:
        LOG_DBG("Unknown obj type specified: 0x%02X", type);
        break;
    }

    return Trackable::NullTrackablePtr;
}


SharedTrackablePtr
ObjRsrc::AllocObj(Trackable::ObjType type, string lookupName)
{
    if (lookupName.length() == 0) {
        LOG_DBG("Parameter lookupName has no value");
        return Trackable::NullTrackablePtr;
    }

    SharedTrackablePtr newObj = AllocWorker(type);
    if (newObj == Trackable::NullTrackablePtr) {
        LOG_DBG("System unable to create object from heap");
        return Trackable::NullTrackablePtr;
    }

    // Store this allocated object in a more permanent container
    pair<TrackableMap::iterator, bool> result;
    result = mObjGrpLife.insert(TrackablePair(lookupName, newObj));
    if (result.second == false) {
        LOG_DBG("Created object with collisions in lookupName: %s",
            lookupName.c_str());
        return Trackable::NullTrackablePtr;
    }

    return GetObj(lookupName);
}


SharedTrackablePtr
ObjRsrc::GetObj(string lookupName)
{
    TrackableMap::iterator item;

    item = mObjGrpLife.find(lookupName);
    if (item == mObjGrpLife.end()) {
        LOG_DBG("Object lookup name %s was not found", lookupName.c_str());
        return SharedTrackablePtr();  // (SharedTrackablePtr->expired() == true)
    }
    return (*item).second;
}


void
ObjRsrc::FreeAllObj()
{
    /** @note
     * By removing all pointers contained within the container it will destroy
     * the contained share_ptr's and thus the objects being pointed to. This, of
     * course, assumes that all other shared_ptr's to those objects have been
     * destroyed. This should be the case since the resource manager creates
     * objects on behalf of tests and all test objects within a group are
     * deleted after they complete, thus removing localized share_ptr's.
     */
    LOG_NRM("Group level resources are being freed: %ld", mObjGrpLife.size());
    mObjGrpLife.clear();
}


void
ObjRsrc::FreeAllObjNotASQACQ()
{
    /** @note
     * By removing all pointers contained within the container it will destroy
     * the contained share_ptr's and thus the objects being pointed to. This, of
     * course, assumes that all other shared_ptr's to those objects have been
     * destroyed. This should be the case since the resource manager creates
     * objects on behalf of tests and all test objects within a group are
     * deleted after they complete, thus removing localized share_ptr's.
     */
    TrackableMap::iterator item;
    size_t numB4 = mObjGrpLife.size();

    item = mObjGrpLife.begin();
    while (item == mObjGrpLife.end()) {
        SharedTrackablePtr tPtr = (*item).second;
        Trackable::ObjType obj = tPtr->GetObjType();
        if ((obj != Trackable::OBJ_ACQ) && (obj != Trackable::OBJ_ACQ))
            mObjGrpLife.erase(item);
    }
    LOG_NRM("Group level resources are being freed: %ld",
        (numB4 - mObjGrpLife.size()));
    LOG_NRM("Group level resources remaining: %ld", mObjGrpLife.size());
}


void
ObjRsrc::FreeObj(string lookupName)
{
    TrackableMap::iterator item;

    item = mObjGrpLife.find(lookupName);
    if (item == mObjGrpLife.end()) {
        LOG_DBG("Cannot free obj, name %s was not found", lookupName.c_str());
        return;
    }
    mObjGrpLife.erase(item);
}
