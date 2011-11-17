#ifndef _TRACKABLE_H_
#define _TRACKABLE_H_

#include <boost/shared_ptr.hpp>

class Trackable;    // forward definition
typedef boost::shared_ptr<Trackable>        SharedTrackablePtr;


/**
* This class is the base class for any object which needs to be created by
* the RsrcMngr.
*
* @note This class may throw exceptions.
*/
class Trackable
{
public:
    /**
     * All unique objects which are trackable and thus are allowed to be
     * created and destroyed by RsrcMngr:: must be listed here.
     */
    typedef enum {
        OBJ_MEMBUFFER,
        OBJ_ACQ,
        OBJ_ASQ,
        OBJ_IOCQ,
        OBJ_IOSQ,
        OBJ_IDENTIFY,
        OBJ_CREATEIOCQ,
        OBJ_CREATEIOSQ,
        OBJ_DELETEIOCQ,
        OBJ_DELETEIOSQ,
        OBJTYPE_FENCE           // always must be last element
    } ObjType;

    /// Used to compare for NULL pointers being returned by allocations
    static SharedTrackablePtr NullTrackablePtr;


    /**
     * Those whom derive from this must register what type of object they
     * are to allow the RsrcMngr to control creation and destruction of
     * that object.
     * @param objBeingCreated Pass the type of object this child class is
     */
    Trackable(ObjType objBeingCreated);
    virtual ~Trackable();

    ObjType GetObjType() { return mObjType; }


private:
    Trackable() {}

    ObjType mObjType;
};


#endif
