/*
* Copyright (c) 2012, Intel Corporation.
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

#ifndef _BASESECURITYDEFS_H_
#define _BASESECURITYDEFS_H_


struct SecDPSPUStruct {
    uint16_t    ATACmdCode;
    char        Passphrase[32];
    uint8_t     Pad[4062];
} __attribute__((__packed__));


struct SecEPFLStruct {
    uint16_t    ATACmdCode;
    uint8_t     Pad[4094];
} __attribute__((__packed__));


struct SecEraseUnitStruct {
    uint16_t    ATACmdCode;
    uint8_t     Mode;
    char        Passphrase[32];
    uint8_t     Pad[4061];
} __attribute__((__packed__));


struct SecRcv00Struct {
    uint8_t     Reserved[6];
    uint16_t    SuppSecProtListLen;
    uint8_t     DiscProt;
    uint8_t     ATADevSerPassSecProt;
    uint8_t     Pad[4086];
} __attribute__((__packed__));


struct SecRcv01Struct {
    uint16_t    Reserved;
    uint16_t    CertLen;
    uint8_t     Pad[4092];
} __attribute__((__packed__));


struct SecRcvEFStruct {
    uint8_t     SecStatus;
    uint8_t     Pad[4095];
} __attribute__((__packed__));


////////////////////////////////////////////////////////////////////////////////
//                   REGISTER BIT DEFINITIONS FOLLOW
////////////////////////////////////////////////////////////////////////////////

/// Bit definitions for SECURITY STATUS
typedef enum SECSTSBits {
    SEC_ENABLED = 0x02,
    SEC_LOCKED = 0x04,
    SEC_FROZEN = 0x08,
    SEC_CNT_EXCEEDED = 0x10
} SECSTSBits;


#endif
