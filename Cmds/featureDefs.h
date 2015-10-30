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

#ifndef _FEATUREDEFS_H_
#define _FEATUREDEFS_H_

/*     FeatureID,                FID              */
#define FEATURE_TABLE                                                          \
    ZZ(FID_ARBITRATION,          0x01)                                         \
    ZZ(FID_PWR_MGMT,             0x02)                                         \
    ZZ(FID_LBA_RANGE,            0x03)                                         \
    ZZ(FID_TEMP_THRESHOLD,       0x04)                                         \
    ZZ(FID_ERR_RECOVERY,         0x05)                                         \
    ZZ(FID_VOL_WR_CACHE,         0x06)                                         \
    ZZ(FID_NUM_QUEUES,           0x07)                                         \
    ZZ(FID_IRQ_COALESCING,       0x08)                                         \
    ZZ(FID_IRQ_VEC_CONFIG,       0x09)                                         \
    ZZ(FID_WRITE_ATOMICITY,      0x0a)                                         \
    ZZ(FID_ASYNC_EVENT_CONFIG,   0x0b)                                         \
    ZZ(FID_AUTO_PS_TRANS,        0x0c)                                         \
    ZZ(FID_HOST_MEM_BUF,         0x0d)                                         \
    ZZ(FID_SW_PROGRESS,          0x80)                                         \
    ZZ(FID_HOST_ID,              0x81)                                         \
    ZZ(FID_RESV_NOTIF_MASK,      0x82)                                         \
    ZZ(FID_RESV_PERSIST,         0x83)

#define ZZ(a, b)        a,
typedef enum FeatureID {
    FEATURE_TABLE
    FID_FENCE    // always must be the last element
} FeatureID;
#undef ZZ

extern const uint8_t FID[];

struct HostIDStruct {
    uint8_t     HOSTID;
} __attribute__((__packed__));

struct LBARngTypeStruct {
    uint8_t     Type;
    uint8_t     Attributes;
    uint8_t     RES_2[14];
    uint64_t    SLBA;
    uint64_t    NLB;
    uint64_t    GUID[2];
    uint8_t     RES_30[16];
} __attribute__((__packed__));

typedef enum AutoPSTransStructBits {
    APST_STRUCT_RES_20  =   0xffffffff00000000,
    APST_STRUCT_ITPT    =   0x00000000ffffff00,
    APST_STRUCT_ITPS    =   0x00000000000000f8,
    APST_Struct_RES_0   =   0x0000000000000007
} AutoPSTransStructBits;

typedef enum ArbDW11Bits {
    ARB_DW11_HPW        = 0xff000000,
    ARB_DW11_MPW        = 0x00ff0000,
    ARB_DW11_LPW        = 0x0000ff00,
    ARB_DW11_RES_3      = 0x000000f8,
    ARB_DW11_AB         = 0x00000007
} ArbDW11Bits;

typedef enum PwrMgmtDW11Bits {
    PM_DW11_RES_8       = 0xffffff00,
    PM_DW11_WH          = 0x000000e0,
    PM_DW11_PS          = 0x0000001f
} PwrMgmtDW11Bits;

typedef enum LBARngTypeDW11Bits {
    LRT_DW11_RES_6      = 0xffffffc0,
    LRT_DW11_NUM        = 0x0000003f
} LBARngTypeDW11Bits;

typedef enum TempThreshDW11Bits {
    TT_DW11_RES_16      = 0xffc00000,
    TT_DW11_THSEL       = 0x00300000,
    TT_DW11_TMPSEL      = 0x000f0000,
    TT_DW11_TMPTH       = 0x0000ffff
} TempThreshDW11Bits;

typedef enum ErrRecoveryDW11Bits {
    ER_DW11_TLER        = 0x0000ffff,
    ER_DW11_DULBE       = 0x00010000,
    ER_DW11_RES_11      = 0xfffe0000
} ErrRecoveryDW11Bits;

typedef enum VolatileWrCacheDW11Bits {
    VWC_DW11_RES_1      = 0xfffffffe,
    VWC_DW11_WCE        = 0x00000001
} VolatileWrCacheDW11Bits;

typedef enum NumQDW11Bits {
    NQ_DW11_NCQR        = 0xffff0000,
    NQ_DW11_NSQR        = 0x0000ffff
} NumQDW11Bits;

typedef enum IntrptCoalesceDW11Bits {
    IC_DW11_RES_10      = 0xffff0000,
    IC_DW11_TIME        = 0x0000ff00,
    IC_DW11_THR         = 0x000000ff
} IntrptCoalesceDW11Bits;

typedef enum IntrptVectorConfigDW11Bits {
    IVC_DW11_RES_11     = 0xfffe0000,
    IVC_DW11_CD         = 0x00010000,
    IVC_DW11_IV         = 0x0000ffff
} IntrptVectorConfigDW11Bits;

typedef enum WrAtomNormDW11Bits {
    WAN_DW11_RES_1      = 0xfffffffe,
    WAN_DW11_DN         = 0x00000001
} WrAtomNormDW11Bits;

typedef enum AsyncEventConfigDW11Bits {
    AEC_DW11_RES_a      = 0xfffffc00,
    AEC_DW11_FAN        = 0x00000200,
    AEC_DW11_NAN        = 0x00000100,
    AEC_DW11_SMART      = 0x000000ff
} AsyncEventConfigDW11Bits;

typedef enum AutoPSTransDW11Bits {
    APST_DW11_RES_1     = 0xfffffffe,
    APST_DW11_APSTE     = 0x00000001
} AutoPSTransDW11Bits;

typedef enum HostMemBufDW11Bits {
    HMB_DW11_RES_2      = 0xfffffffc,
    HMB_DW11_MR         = 0x00000002,
    MHB_DW11_EHM        = 0x00000001
} HostMemBufDW11Bits;

typedef enum HostMemBufDW12Bits {
    HMB_DW12_HSIZE      = 0xffffffff
} HostMemBufDW12Bits;

typedef enum HostMemBufDW13Bits {
    HMB_DW13_HMDLLA     = 0xfffffff0,
    HMB_DW13_RES_0      = 0x0000000f
} HostMemBufDW13Bits;

typedef enum HostMemBufDW14Bits {
    HMB_DW14_HMDLUA     = 0xffffffff
} HostMemBufDW14Bits;

typedef enum HostMemBufDW15Bits {
    HMB_DW15_HMDLEC     = 0xffffffff
} HostMemBufDW15Bits;

typedef enum SWProgMarkerDW11Bits {
    SWPM_DW11_RES_8     = 0xffffff00,
    SWPM_DW11_PBSLC     = 0x000000ff
} SWProgMarkerDW11Bits;

/* Host Identifier does not use DW11 */

typedef enum ResNotifMaskDW11Bits {
    RNM_DW11_RES_4      = 0xfffffff0,
    RNM_DW11_RESPRE     = 0x00000008,
    RNM_DW11_RESREL     = 0x00000004,
    RNM_DW11_REGPRE     = 0x00000002,
    RNM_DW11_RES_0      = 0x00000001
} ResNotifConfigDW11Bits;

typedef enum ResPersistDW11Bits {
    RP_DW11_RES_1       = 0xfffffffe,
    RP_DW11_PTPL        = 0x00000001
} ResPersistDW11Bits;

typedef enum NumQDW0Bits {
    NQ_DW11_NCQA     = 0xffff0000,
    NQ_DW11_NSQA     = 0x0000ffff
} NumQDW0Bits;

typedef enum SelDW10Byte {
	SEL_CURRENT     = 0x0,
	SEL_DEFAULT     = 0x1,
	SEL_SAVED       = 0x2,
	SEL_SUPPORTED   = 0x3,
	SEL_RES         = 0x4
} SelDW10Byte;

typedef enum GetDW0 {
	GET_DW0_SAVEABLE        = 0x1,
	GET_DW0_IND_NAMSPC      = 0x2,
	GET_DW0_CHANGEABLE      = 0x4
} GetDW0;

#endif
