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

#ifndef _RESERVATIONACQUIRE_H_
#define _RESERVATIONACQUIRE_H_

#include "cmd.h"

class ReservationAcquire;    // forward definition
typedef boost::shared_ptr<ReservationAcquire>             SharedReservationAcquirePtr;
typedef boost::shared_ptr<const ReservationAcquire>       ConstSharedReservationAcquirePtr;
#define CAST_TO_RESERVATINOACQUIRE(shared_trackable_ptr)  \
        boost::shared_polymorphic_downcast<ReservationAcquire>(shared_trackable_ptr);


/**
* This class implements the dataset mgmt nvm cmd
*
* @note This class may throw exceptions.
*/
class ReservationAcquire : public Cmd
{
public:
    ReservationAcquire();
    virtual ~ReservationAcquire();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedReservationAcquirePtr NullReservationAcquirePtr;
    static const uint8_t Opcode;

    /**
     * @param nr Pass the value, it is 0-based per the spec.
     */
    void    SetRTYPE(uint8_t);
    uint8_t GetRTYPE() const;

    /**
     * @param ad Pass the bit value
     */
    void SetIEKEY(bool);
    bool GetIEKEY() const;

    /**
     * @param idw Pass the bit value
     */
    void SetRACQA(uint8_t);
    uint8_t GetRACQA() const;
};


#endif
