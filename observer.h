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

#ifndef _OBSERVER_H_
#define _OBSERVER_H_

#include <vector>


/**
* This class implements the observer component of the subject/observer design
* pattern.
* @note This class does not throw exceptions.
*/
template <class TOSubject, class TOData>
class Observer
{
public:
  Observer() {}
  virtual ~Observer() {}
  virtual void Update(TOSubject *subject, const TOData &data) = 0;
};


/**
 * This class observes state changes
 * @note This class does not throw exceptions.
 */
template<class TSOData>
class StateObserver : public Observer<void, TSOData>
{
public:
    StateObserver() {}
    virtual ~StateObserver() {}
    virtual void Update(void *, const TSOData &data) { Update(data); }
    virtual void Update(const TSOData &data) = 0;
};


/**
 * This class observes another object, i.e. a pure observation technique
 * @note This class does not throw exceptions.
 */
template<class TOOSubject>
class PureObserver : public Observer<TOOSubject, bool>
{
public:
    PureObserver() {}
    virtual ~PureObserver() {}
    virtual void Update(TOOSubject *subject, const bool &) { Update(subject); }
    virtual void Update(TOOSubject *subject) = 0;
};


#endif
