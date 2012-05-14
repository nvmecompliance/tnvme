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

#ifndef _READ_H_
#define _READ_H_

#include "cmd.h"


class Read;    // forward definition
typedef boost::shared_ptr<Read>             SharedReadPtr;
typedef boost::shared_ptr<const Read>       ConstSharedReadPtr;
#define CAST_TO_READ(shared_trackable_ptr)  \
        boost::shared_polymorphic_downcast<Read>(shared_trackable_ptr);


/**
* This class implements the read admin cmd
*
* @note This class may throw exceptions.
*/
class Read : public Cmd
{
public:
    Read();
    virtual ~Read();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedReadPtr NullReadPtr;
    static const uint8_t Opcode;

    /**
     * Set the Starting Logical Block Address (SLBA).
     * @param lba Pass the LBA to start writing data
     */
    void     SetSLBA(uint64_t lba);
    uint64_t GetSLBA() const;

    /**
     * Set the Limited Retry (LR)
     * @param lr Pass true to set, otherwise false
     */
    void SetLR(bool lr);
    bool GetLR() const;

    /**
     * Set the Force Unit Access (FUA)
     * @param fua Pass true to set, otherwise false
     */
    void SetFUA(bool fua);
    bool GetFUA() const;

    /**
     * Set the Protection Information Field (PRINFO)
     * @param prinfo Pass any value which can be set in 4 bits
     */
    void    SetPRINFO(uint8_t prinfo);
    uint8_t GetPRINFO() const;

    /**
     * Set the Number of Logical Blocks (NLB)
     * @param nlb Pass the new value to set
     */
    void     SetNLB(uint16_t nlb);
    uint16_t GetNLB() const;

    /**
     * Set the Data Set Management (DSM) Incompressible
     * @param incompress Pass true to set, otherwise false
     */
    void SetDSMIncompress(bool incompress);
    bool GetDSMIncompress() const;

    /**
     * Set the Data Set Management (DSM) Sequential Request
     * @param seqReq Pass true to set, otherwise false
     */
    void SetDSMSeqRequest(bool seqReq);
    bool GetDSMSeqRequest() const;

    /**
     * Set the Data Set Management (DSM) Access Latency
     * @param accessLat Pass any value which can be set in 2 bits
     */
    void    SetDSMAccessLatent(uint8_t accessLat);
    uint8_t GetDSMAccessLatent() const;

    /**
     * Set the Data Set Management (DSM) Access Frequency
     * @param accessFreq Pass any value which can be set in 4 bits
     */
    void    SetDSMAccessFreq(uint8_t accessFreq);
    uint8_t GetDSMAccessFreq() const;

    /**
     * Set the Expected Initial Logical Block Reference Tag (EILBRT)
     * @param eilbrt Pass the new value to set
     */
    void     SetEILBRT(uint32_t eilbrt);
    uint32_t GetEILBRT() const;

    /**
     * Set the Expected Logical Block Application Tag Mask (ELBATM)
     * @param elbatm Pass the new value to set
     */
    void     SetELBATM(uint16_t elbatm);
    uint16_t GetELBATM() const;

    /**
     * Set the Expected Logical Block Application Tag (ELBAT)
     * @param elbat Pass the new value to set
     */
    void     SetELBAT(uint16_t elbat);
    uint16_t GetELBAT() const;
};


#endif
