#ifndef _TRACKABLE_H_
#define _TRACKABLE_H_



/**
* This class is the base class for any object which needs to be created by
* the RsrcMngr::
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
        OBJTYPE_FENCE           // always must be last element
    } ObjType;

    typedef enum {
        LIFE_TEST,              // Deleted/freed after a test completes
        LIFE_GROUP,             // Deleted/freed after a group completes
        LIFETIME_FENCE          // always must be last element
    } Lifetime;


    /**
     * Those whom derive from this must register what type of object they
     * are to allow the RsrcMngr:: to control creation and destruction of
     * that object.
     * @param objBeingCreated Pass the type of object this child class is
     * @param objLife Pass the lifetime of the object being created
     * @param ownByRsrcMngr Pass true if the RsrcMngr created this obj,
     *      otherwise false.
     */
    Trackable(ObjType objBeingCreated, Lifetime objLife, bool ownByRsrcMngr);
    virtual ~Trackable();

    ObjType GetObjType() { return mObjType; }
    Lifetime GetObjLife() { return mObjLife; }
    bool GetOwnByRsrcMngr() { return mOwnByRsrcMngr; }


private:
    Trackable() {}

    ObjType mObjType;
    Lifetime mObjLife;
    bool mOwnByRsrcMngr;
};


#endif
