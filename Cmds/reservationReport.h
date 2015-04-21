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

#ifndef _RESERVATIONREPORT_H_
#define _RESERVATIONREPORT_H_

#include "cmd.h"

class ReservationReport;    // forward definition
typedef boost::shared_ptr<ReservationReport>             SharedReservationReportPtr;
typedef boost::shared_ptr<const ReservationReport>       ConstSharedReservationReportPtr;
#define CAST_TO_RESERVATINOREPORT(shared_trackable_ptr)  \
        boost::shared_polymorphic_downcast<ReservationReport>(shared_trackable_ptr);


/**
* This class implements the dataset mgmt nvm cmd
*
* @note This class may throw exceptions.
*/
class ReservationReport : public Cmd
{
public:
    ReservationReport();
    virtual ~ReservationReport();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedReservationReportPtr NullReservationReportPtr;
    static const uint8_t Opcode;

    /**
     * @param nr Pass the value, it is 0-based per the spec.
     */
    void    SetNUMD(uint32_t);
    uint32_t GetNUMD() const;

};


#endif
