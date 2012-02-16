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
    Subject(TSubject *subject, TData initState = TData(), bool delta = false) :
        mSubject(subject),
        mCurState(initState),
        mDeltaCheck(delta) { mFirstTime = true; }
    virtual ~Subject() {}

    void Attach(Observer<TSubject, TData> &observer)
        { mObservers.push_back(&observer); }

    TData GetCurrentState() const { return mCurState; }

    void Notify(const TData& newSt)
    {
        bool doUpdate = true;
        if (mDeltaCheck) {
            if ((mCurState == newSt) && (mFirstTime == false))
                doUpdate = false;
        }
        mFirstTime = false;
        if (doUpdate) {
            mCurState = newSt;
            LOG_DBG("Subject notifying observers of event");
            for (size_t i = 0; i < mObservers.size(); i++)
                (mObservers[i])->Update(static_cast<TSubject *>(this), newSt);
        }
    }

private:
    TSubject *mSubject;
    TData mCurState;
    std::vector<Observer<TSubject, TData> *> mObservers;

    /// Update() all observers on 1st Notify regardless of all other options
    bool mFirstTime;
    /// Update() all observers only when the state changes value
    bool mDeltaCheck;
};


/**
 * This class implements a state subject
 * @note This class does not throw exceptions.
 */
template<class TSSData>
class StateSubject : public Subject<void, TSSData>
{
public:
    StateSubject(TSSData initState = TSSData()) :
        Subject<void, TSSData>(NULL, initState) {}
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
