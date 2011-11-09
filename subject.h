#ifndef _SUBJECT_H_
#define _SUBJECT_H_

#include <vector>
#include "observer.h"


/**
* This class implements the subject component of the subject/observer design
* pattern.
* @note This class does not throw exceptions.
*/
template <class TSubject, class TData>
class Subject
{
public:
    Subject(TSubject *subject, TData initialState = TData()) :
        mSubject(subject),
        mCurState(initialState) { firstTime = true; }
    virtual ~Subject() {}

    void Attach(Observer<TSubject, TData> &observer)
        { mObservers.push_back(&observer); }

    TData GetCurrentState() const { return mCurState; }

    void Notify(const TData& newSt)
    {
        if ((mCurState != newSt) || firstTime) {
            firstTime = false;
            mCurState = newSt;
            LOG_DBG("Subject notifying observers of event");
            for (size_t i = 0; i < mObservers.size(); i++)
                (mObservers[0])->Update(static_cast<TSubject *>(this), newSt);
        }
    }

private:
    TSubject *mSubject;
    TData mCurState;
    std::vector<Observer<TSubject, TData> *> mObservers;

    /// Only Update() Observers when state changed, but always on 1st state
    bool firstTime;
};


/**
 * This class implements a state subject
 * @note This class does not throw exceptions.
 */
template<class TSSData>
class StateSubject : public Subject<void, TSSData>
{
public:
    StateSubject(TSSData initialState = TSSData()) :
        Subject<void, TSSData>(NULL, initialState) {}
    virtual ~StateSubject() {}

};


/**
 * This class implements an object subject, i.e. a pure subject technique
 * @note This class does not throw exceptions.
 */
template<class TOSSubject>
class PureSubject : public Subject<TOSSubject, bool>
{
public:
    PureSubject(TOSSubject *subject) :
        Subject<TOSSubject, bool>(subject, true) {}
    virtual ~PureSubject() {}

};


#endif
