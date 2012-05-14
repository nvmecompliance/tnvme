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

#ifndef _FORMATNVM_H_
#define _FORMATNVM_H_

#include "cmd.h"


class FormatNVM;    // forward definition
typedef boost::shared_ptr<FormatNVM>             SharedFormatNVMPtr;
typedef boost::shared_ptr<const FormatNVM>       ConstSharedFormatNVMPtr;
#define CAST_TO_FORMATNVM(shared_trackable_ptr)  \
        boost::shared_polymorphic_downcast<FormatNVM>(shared_trackable_ptr);


/**
* This class implements the Format NVM admin cmd.
*
* @note This class may throw exceptions.
*/
class FormatNVM : public Cmd
{
public:
    FormatNVM();
    virtual ~FormatNVM();

    /// Used to compare for NULL pointers being returned by allocations
    static SharedFormatNVMPtr NullFormatNVMPtr;
    static const uint8_t Opcode;

    enum {
        SES_NOP,
        SES_USER_DATA,
        SES_CRYPTO_ERASE
    };

    /**
     * Set the Secure Erase Settings field (SES)
     * @param ses Pass any value which can be set in DW10.SES bitfield
     */
    void    SetSES(uint8_t ses);
    uint8_t GetSES() const;

    /**
     * Set the Protection Information Location field (PIL)
     * @param pil Pass the new value of the PIL field
     */
    void SetPIL(bool pil);
    bool GetPIL() const;

    /**
     * Set the Protection Information field (PI)
     * @param pi Pass any value which can be set in DW10.PI bitfield
     */
    void    SetPI(uint8_t pi);
    uint8_t GetPI() const;

    /**
     * Set the Meta data Settings field (MS)
     * @param ms Pass the new value of the MS field
     */
    void SetMS(bool ms);
    bool GetMS() const;

    /**
     * Set the LBA Format field (LBAF)
     * @param lbaf Pass any value which can be set in DW10.LBAF bitfield
     */
    void SetLBAF(uint8_t lbaf);
    uint8_t GetLBAF() const;
};


#endif
