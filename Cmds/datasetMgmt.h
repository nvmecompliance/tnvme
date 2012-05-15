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

#ifndef _DATASETMGMT_H_
#define _DATASETMGMT_H_

#include "cmd.h"


struct CtxAttrib {
    uint16_t AF        : 4;      // access frequency
    uint16_t AL        : 2;      // access latency
    uint16_t reserved0 : 2;
    uint16_t SR        : 1;      // sequential read range
    uint16_t SW        : 1;      // sequential write range
    uint16_t WP        : 1;      // write prepared
    uint16_t reserved1 : 13;
    uint16_t CAS       : 8;      // cmd access size
} __attribute__((__packed__));

struct RangeDef {
    CtxAttrib ctxAttrib;
    uint32_t  length;
    uint64_t  slba;
} __attribute__((__packed__));


class DatasetMgmt;    // forward definition
typedef boost::shared_ptr<DatasetMgmt>             SharedDatasetMgmtPtr;
typedef boost::shared_ptr<const DatasetMgmt>       ConstSharedDatasetMgmtPtr;
#define CAST_TO_DATASETMGMT(shared_trackable_ptr)  \
        boost::shared_polymorphic_downcast<DatasetMgmt>(shared_trackable_ptr);


/**
* This class implements the dataset mgmt nvm cmd
*
* @note This class may throw exceptions.
*/
class DatasetMgmt : public Cmd
{
public:
    DatasetMgmt();
    virtual ~DatasetMgmt();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedDatasetMgmtPtr NullDatasetMgmtPtr;
    static const uint8_t Opcode;

    /**
     * Set the Number of Ranges field (NR)
     * @param nr Pass the value, it is 0-based per the spec.
     */
    void    SetNR(uint8_t nr);
    uint8_t GetNR() const;

    /**
     * Set the Attribute-Deallocate field (AD)
     * @param ad Pass the bit value
     */
    void SetAD(bool ad);
    bool GetAD() const;

    /**
     * Set the Attribute-Integral Dataset for Write field (IDW)
     * @param idw Pass the bit value
     */
    void SetIDW(bool idw);
    bool GetIDW() const;

    /**
     * Set the Attribute-Integral Dataset for Read field (IDR)
     * @param idr Pass the bit value
     */
    void SetIDR(bool idr);
    bool GetIDR() const;
};


#endif
