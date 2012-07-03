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

#ifndef _FWIMGDNLD_H_
#define _FWIMGDNLD_H_

#include "cmd.h"


class FWImgDnld;    // forward definition
typedef boost::shared_ptr<FWImgDnld>              SharedFWImgDnldPtr;
typedef boost::shared_ptr<const FWImgDnld>        ConstSharedFWImgDnldPtr;
#define CAST_TO_FWIMGDNLD(shared_trackable_ptr)   \
        boost::shared_polymorphic_downcast<FWImgDnld>(shared_trackable_ptr);


/**
* This class implements the FW Activate admin cmd
*
* @note This class may throw exceptions.
*/
class FWImgDnld : public Cmd
{
public:
    FWImgDnld();
    virtual ~FWImgDnld();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedFWImgDnldPtr NullFWImgDnldPtr;
    static const uint8_t Opcode;

    /**
     * Set the Number of Dwords (NUMD)
     * @param numd Pass the number of DW's = sizeof(img to download)
     */
    void SetNUMD(uint32_t numd);
    uint32_t GetNUMD() const;


    /**
     * Set the Offset (OFST)
     * @param ofst Pass the offset address where to program this data segment
     */
    void SetOFST(uint32_t ofst);
    uint32_t GetOFST() const;
};


#endif
