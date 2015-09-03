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

#ifndef _IDENTIFYDEFS_H_
#define _IDENTIFYDEFS_H_


struct IdentifyDataType {
    uint16_t    offset;        // byte offset into the returned data struct
    uint16_t    length;        // number of bytes this field consumes
    bool        mandatory;     // whether the field is mandatory or optional
    SpecRev     specRev;       // the specification revision of the field
    const char *desc;          // this fields formal description
};


typedef enum {
	NS_BARE,    // no meta data, nor E2E data
	NS_METAI,   // META data Interleaved (METAI) in LBA payload
	NS_METAS,   // META data Separate (METAS) buffer
	NS_E2EI,    // E2E data Interleaved (E2EI) in LBA payload
	NS_E2ES,    // E2E data Separate (E2ES) buffer
	NS_UNKNOWN, // failure type case
} NamspcType;

/*     IdCtrlrCap,          offset, length, m/o,   specRev,     desc                                   */
#define IDCTRLRCAP_PSD_TABLE \
    ZZ(IDCTRLRCAP_PSD0,     2048,   32,     true,  SPECREV_10b, "Power State 0 Desc (PSD0)")            \
    ZZ(IDCTRLRCAP_PSD1,     2080,   32,     false, SPECREV_10b, "Power State 1 Desc (PSD1)")            \
    ZZ(IDCTRLRCAP_PSD2,     2112,   32,     false, SPECREV_10b, "Power State 2 Desc (PSD2)")            \
    ZZ(IDCTRLRCAP_PSD3,     2144,   32,     false, SPECREV_10b, "Power State 3 Desc (PSD3)")            \
    ZZ(IDCTRLRCAP_PSD4,     2176,   32,     false, SPECREV_10b, "Power State 4 Desc (PSD4)")            \
    ZZ(IDCTRLRCAP_PSD5,     2208,   32,     false, SPECREV_10b, "Power State 5 Desc (PSD5)")            \
    ZZ(IDCTRLRCAP_PSD6,     2240,   32,     false, SPECREV_10b, "Power State 6 Desc (PSD6)")            \
    ZZ(IDCTRLRCAP_PSD7,     2272,   32,     false, SPECREV_10b, "Power State 7 Desc (PSD7)")            \
    ZZ(IDCTRLRCAP_PSD8,     2304,   32,     false, SPECREV_10b, "Power State 8 Desc (PSD8)")            \
    ZZ(IDCTRLRCAP_PSD9,     2336,   32,     false, SPECREV_10b, "Power State 9 Desc (PSD9)")            \
    ZZ(IDCTRLRCAP_PSD10,    2368,   32,     false, SPECREV_10b, "Power State 10 Desc (PSD10)")          \
    ZZ(IDCTRLRCAP_PSD11,    2400,   32,     false, SPECREV_10b, "Power State 11 Desc (PSD11)")          \
    ZZ(IDCTRLRCAP_PSD12,    2432,   32,     false, SPECREV_10b, "Power State 12 Desc (PSD12)")          \
    ZZ(IDCTRLRCAP_PSD13,    2464,   32,     false, SPECREV_10b, "Power State 13 Desc (PSD13)")          \
    ZZ(IDCTRLRCAP_PSD14,    2496,   32,     false, SPECREV_10b, "Power State 14 Desc (PSD14)")          \
    ZZ(IDCTRLRCAP_PSD15,    2528,   32,     false, SPECREV_10b, "Power State 15 Desc (PSD15)")          \
    ZZ(IDCTRLRCAP_PSD16,    2560,   32,     false, SPECREV_10b, "Power State 16 Desc (PSD16)")          \
    ZZ(IDCTRLRCAP_PSD17,    2592,   32,     false, SPECREV_10b, "Power State 17 Desc (PSD17)")          \
    ZZ(IDCTRLRCAP_PSD18,    2624,   32,     false, SPECREV_10b, "Power State 18 Desc (PSD18)")          \
    ZZ(IDCTRLRCAP_PSD19,    2656,   32,     false, SPECREV_10b, "Power State 19 Desc (PSD19)")          \
    ZZ(IDCTRLRCAP_PSD20,    2688,   32,     false, SPECREV_10b, "Power State 20 Desc (PSD20)")          \
    ZZ(IDCTRLRCAP_PSD21,    2720,   32,     false, SPECREV_10b, "Power State 21 Desc (PSD21)")          \
    ZZ(IDCTRLRCAP_PSD22,    2752,   32,     false, SPECREV_10b, "Power State 22 Desc (PSD22)")          \
    ZZ(IDCTRLRCAP_PSD23,    2784,   32,     false, SPECREV_10b, "Power State 23 Desc (PSD23)")          \
    ZZ(IDCTRLRCAP_PSD24,    2816,   32,     false, SPECREV_10b, "Power State 24 Desc (PSD24)")          \
    ZZ(IDCTRLRCAP_PSD25,    2848,   32,     false, SPECREV_10b, "Power State 25 Desc (PSD25)")          \
    ZZ(IDCTRLRCAP_PSD26,    2880,   32,     false, SPECREV_10b, "Power State 26 Desc (PSD26)")          \
    ZZ(IDCTRLRCAP_PSD27,    2912,   32,     false, SPECREV_10b, "Power State 27 Desc (PSD27)")          \
    ZZ(IDCTRLRCAP_PSD28,    2944,   32,     false, SPECREV_10b, "Power State 28 Desc (PSD28)")          \
    ZZ(IDCTRLRCAP_PSD29,    2976,   32,     false, SPECREV_10b, "Power State 29 Desc (PSD29)")          \
    ZZ(IDCTRLRCAP_PSD30,    3008,   32,     false, SPECREV_10b, "Power State 30 Desc (PSD30)")          \
    ZZ(IDCTRLRCAP_PSD31,    3040,   32,     false, SPECREV_10b, "Power State 31 Desc (PSD31)")

/* m=true, o=false */
/*     IdCtrlrCap,          offset, length, m/o,   specRev,     desc                                   */
#define IDCTRLRCAP_TABLE                                                            \
    ZZ(IDCTRLRCAP_VID,      0,      2,      true,  SPECREV_10b, "PCI Vendor ID (VID)")                  \
    ZZ(IDCTRLRCAP_SSVID,    2,      2,      true,  SPECREV_10b, "PCI Subsystem Vendor ID (SSVID)")      \
    ZZ(IDCTRLRCAP_SN,       4,      20,     true,  SPECREV_10b, "Serial Number (SN)")                   \
    ZZ(IDCTRLRCAP_MN,       24,     40,     true,  SPECREV_10b, "Model Number (MN)")                    \
    ZZ(IDCTRLRCAP_FR,       64,     8,      true,  SPECREV_10b, "Firmware Revision (FR)")               \
    ZZ(IDCTRLRCAP_RAB,      72,     1,      true,  SPECREV_10b, "Recommend Arbitration Burst (RAB)")    \
    ZZ(IDCTRLRCAP_IEEE,     73,     3,      true,  SPECREV_10b, "IEEE OUI Identifier (IEEE)")           \
    ZZ(IDCTRLRCAP_MIC,      76,     1,      true,  SPECREV_10b, "Multi-Interface Capabilities (MIC)")  \
    ZZ(IDCTRLRCAP_CMIC,     76,     1,      false, SPECREV_11,  "Ctrlr Multipath IO & Namspc Share Cap (CMIC)")  \
    ZZ(IDCTRLRCAP_MDTS,     77,     1,      true,  SPECREV_10b, "Maximum Data Xfer Size (MDTS)")        \
    ZZ(IDCTRLRCAP_RES4E,    78,     178,    true,  SPECREV_10b, "Reserved area @ 0x4e")                 \
    ZZ(IDCTRLRCAP_CNTLID,   78,     2,      false, SPECREV_11,  "Controller ID (CNTLID)")               \
    ZZ(IDCTRLRCAP_RES50,    80,     176,    true,  SPECREV_11,  "Reserved area @ 0x50")                 \
    ZZ(IDCTRLRCAP_VER,      80,     4,      false, SPECREV_12,  "NVMe Spec Version (VER)")              \
    ZZ(IDCTRLRCAP_RTD3R,    84,     4,      false, SPECREV_12,  "Resume Latency (RTD3R)")               \
    ZZ(IDCTRLRCAP_RTD3E,    88,     4,      false, SPECREV_12,  "Entry Latency (RTD3E)")                \
    ZZ(IDCTRLRCAP_OAES,     92,     4,      false, SPECREV_12,  "Opt Async Events Supported)")          \
    ZZ(IDCTRLRCAP_RES60,    96,     160,    false, SPECREV_12,  "Reserved area @ 0x60")                 \
    ZZ(IDCTRLRCAP_OACS,     256,    2,      true,  SPECREV_10b, "Optional Admin Cmd Support (OACS)")    \
    ZZ(IDCTRLRCAP_ACL,      258,    1,      true,  SPECREV_10b, "Abort Cmd Limit (ACL)")                \
    ZZ(IDCTRLRCAP_AERL,     259,    1,      true,  SPECREV_10b, "Async Event Req Limit (AERL)")         \
    ZZ(IDCTRLRCAP_FRMW,     260,    1,      true,  SPECREV_10b, "Firmware Updates (FRMW)")              \
    ZZ(IDCTRLRCAP_LPA,      261,    1,      true,  SPECREV_10b, "Log Page Atribute (LPA)")              \
    ZZ(IDCTRLRCAP_ELPE,     262,    1,      true,  SPECREV_10b, "Error Log Page Entries (ELPE)")        \
    ZZ(IDCTRLRCAP_NPSS,     263,    1,      true,  SPECREV_10b, "Number of Power States Support (NPSS)")\
    ZZ(IDCTRLRCAP_RES108,   264,    248,    true,  SPECREV_10b, "Reserved area @ 0x108")                \
    ZZ(IDCTRLRCAP_AVSCC,    264,    1,      true,  SPECREV_11,  "Admin Vendor Spec Cmd Config (AVSCC)") \
    ZZ(IDCTRLRCAP_APSTA,    265,    1,      false, SPECREV_11,  "Autonomous PS Transition Attr (APSTA)")\
    ZZ(IDCTRLRCAP_RES10A,   266,    246,    true,  SPECREV_11,  "Reserved area @ 0x10A")                \
    ZZ(IDCTRLRCAP_WCTEMP,   266,    2,      false, SPECREV_12,  "Warn Composite Temp Thresh (WCTEMP)")  \
    ZZ(IDCTRLRCAP_CCTEMP,   268,    2,      false, SPECREV_12,  "Crit Composite Temp Thresh (CCTEMP)")  \
    ZZ(IDCTRLRCAP_MTFA,     270,    2,      false, SPECREV_12,  "Max Time for FW Activation (MTFA)")    \
    ZZ(IDCTRLRCAP_HMPRE,    272,    4,      false, SPECREV_12,  "Host Mem Buf Preferred Size (HMPRE)")  \
    ZZ(IDCTRLRCAP_HMMIN,    276,    4,      false, SPECREV_12,  "Host Mem Buffer Minimum Size (HMMIN)") \
    ZZ(IDCTRLRCAP_TNVMCAP_LOWER,280,8,      false, SPECREV_12,        "TNVMCAP_LOWER")\
    ZZ(IDCTRLRCAP_TNVMCAP_UPPER,288,8,      false, SPECREV_12,        "TNVMCAP_UPPER")\
    ZZ(IDCTRLRCAP_UNVMCAP_LOWER,296,8,      false, SPECREV_12,        "UNVMCAP_LOWER")\
    ZZ(IDCTRLRCAP_UNVMCAP_UPPER,304,8,      false, SPECREV_12,        "UNVMCAP_UPPER")\
    ZZ(IDCTRLRCAP_RPMBS,    312,    4,      false, SPECREV_12,  "Replay Protected Mem Blk Supp (RPMBS)")\
    ZZ(IDCTRLRCAP_RES13C,   316,    196,    false, SPECREV_12,  "Reserved area @ 0x13c")                \
    ZZ(IDCTRLRCAP_SQES,     512,    1,      true,  SPECREV_10b, "Submission Q Entry Size (SQES)")       \
    ZZ(IDCTRLRCAP_CQES,     513,    1,      true,  SPECREV_10b, "Completion Q Entry Size (CQES)")       \
    ZZ(IDCTRLRCAP_RES202,   514,    2,      true,  SPECREV_10b, "Reserved area @ 0x202")                \
    ZZ(IDCTRLRCAP_NN,       516,    4,      true,  SPECREV_10b, "Number of Namespaces (NN)")            \
    ZZ(IDCTRLRCAP_ONCS,     520,    2,      true,  SPECREV_10b, "Optional NVM Cmd Support (ONCS)")      \
    ZZ(IDCTRLRCAP_FUSES,    522,    2,      true,  SPECREV_10b, "Fused Operation Suppot (FUSES)")       \
    ZZ(IDCTRLRCAP_FNA,      524,    1,      true,  SPECREV_10b, "Format NVM Attribute (FNA)")           \
    ZZ(IDCTRLRCAP_VWC,      525,    1,      true,  SPECREV_10b, "Volatile Write Cache (VWC)")           \
    ZZ(IDCTRLRCAP_AWUN,     526,    2,      true,  SPECREV_10b, "Atomic Write Unit Normal (AWUN)")      \
    ZZ(IDCTRLRCAP_AWUPF,    528,    2,      true,  SPECREV_10b, "Atomic Write Unit Power Fail (AWUPF)") \
    ZZ(IDCTRLRCAP_RES212,   530,    174,    true,  SPECREV_10b, "Reserved area @ 0x212")                \
    ZZ(IDCTRLRCAP_NVSCC,    530,    1,      true,  SPECREV_11,  "NVM Vendor Spec Cmd Config (NVSCC)")   \
    ZZ(IDCTRLRCAP_RES213,   531,    1,      true,  SPECREV_11,  "Reserved area @ 0x213")                \
    ZZ(IDCTRLRCAP_ACWU,     532,    2,      false, SPECREV_11,  "Atomic Compare & Write Unit (ACWU)")   \
    ZZ(IDCTRLRCAP_RES216,   534,    2,      true,  SPECREV_11,  "Reserved area @ 0x216")                \
    ZZ(IDCTRLRCAP_SGLS,     536,    4,      false, SPECREV_11,  "SGL Support (SGLS)")                   \
    ZZ(IDCTRLRCAP_RES21C,   540,    164,    true,  SPECREV_11,  "Reserved area @ 0x21C")                \
    ZZ(IDCTRLRCAP_RES2C0,   704,    1344,   true,  SPECREV_10b, "Reserved area @ 2c0")                  \
    IDCTRLRCAP_PSD_TABLE    \
    ZZ(IDCTRLRCAP_VS,       3072,   1024,   false, SPECREV_10b, "Vendor Specific (VS)")

#define ZZ(a, b, c, d, e, f)         a,
typedef enum IdCtrlrCap
{
    IDCTRLRCAP_TABLE
    IDCTRLRCAP_FENCE               // always must be the last element
} IdCtrlrCap;
#undef ZZ

extern IdCtrlrCap IDCTRLRCAP_PSD[];

struct IdPowerStateDesc {
    uint16_t    MP;
    uint16_t    RES;
    uint32_t    ENLAT;
    uint32_t    EXLAT;
    uint8_t     RRT;
    uint8_t     RRL;
    uint8_t     RWT;
    uint8_t     RWL;
    uint64_t    RES_7D[2];
} __attribute__((__packed__));

struct IdPowerStateDescUnpacked {
    uint16_t    MP;
    uint8_t     RES0;
    uint8_t     MPS;
    uint8_t     NOPS;
    uint8_t     RES1;
    uint32_t    ENLAT;
    uint32_t    EXLAT;
    uint8_t     RRT;
    uint8_t     RES2;
    uint8_t     RRL;
    uint8_t     RES3;
    uint8_t     RWT;
    uint8_t     RES4;
    uint8_t     RWL;
    uint64_t    RES5_1;
    uint64_t    RES5_2;
    uint8_t     RES5_3;
};

struct IdCtrlrCapStruct {
	// Cntrlr Caps and Features
    uint16_t    VID;
    uint16_t    SSVID;
    uint8_t     SN[20];
    uint8_t     MN[40];
    uint8_t     FR[8];
    uint8_t     RAB;
    uint8_t     IEEE[3];
    uint8_t     MIC;
    uint8_t     MDTS;    // Byte  77
    uint16_t    CNTLID;  // Bytes 78-79
    uint32_t    VER;     // Bytes 80-83
    uint32_t    RTD3R;   // Bytes 84-87
    uint32_t    RTD3E;   // Bytes 88-91
    uint32_t    OAES;    // Bytes 92-95
    uint8_t     RES_60[160]; //   96-255
    // Admin Cmd Set Attribs
    uint16_t    OACS;   // Bytes 256-257
    uint8_t     ACL;    //       258
    uint8_t     AERL;   //       259
    uint8_t     FRMW;   //
    uint8_t     LPA;
    uint8_t     ELPE;
    uint8_t     NPSS;
    uint8_t		AVSCC; // 1.0c addition. Admin Vendor Spec Cmd Config
    uint8_t     APSTA;   // Byte 265
    uint16_t    WCTEMP;
    uint16_t    CCTEMP;
    uint16_t    MTFA;
    uint32_t    HMPRE;
    uint32_t    HMMIN;
    uint64_t    TNVMCAP_LOWER;
    uint64_t    TNVMCAP_UPPER;
    uint64_t    UNVMCAP_LOWER;
    uint64_t    UNVMCAP_UPPER;
    uint32_t    RPMBS;
    uint8_t     RES_13C[196]; // 316-511
    // NVM Command Set Attributes
    uint8_t     SQES;
    uint8_t     CQES;
    uint16_t    RES_202;
    uint32_t    NN;
    uint16_t    ONCS;
    uint16_t    FUSES;
    uint8_t     FNA;
    uint8_t     VWC;
    uint16_t    AWUN;
    uint16_t    AWUPF; // 1.0c change? Atomic Write Unit Power Fail
    uint8_t		NVSCC; // 1.0c addition. NVM Vendor Spec Cmd Config
    uint8_t     RES_213;
    uint16_t    ACWU;  // 532/533
    uint8_t     RES_216[2];
    uint32_t    SGLS;  // 536-539
    uint8_t     RES_21C[164]; // 540-703
    // I/O Command set Attributes (none yet)
    uint8_t     RES_2C0[1344];
    // Power State Descriptors
    struct IdPowerStateDesc PSD[32];
    // Vendor Specific
    uint8_t     VS[1024];
} __attribute__((__packed__));


/*     IdNamespc,           offset, length, m/o,   specRev,     desc                                   */
#define IDNAMESPC_TABLE                                                             \
    ZZ(IDNAMESPC_NSZE,      0,      8,      true,  SPECREV_10b, "Namespace Size (NSZE)")                \
    ZZ(IDNAMESPC_NCAP,      8,      8,      true,  SPECREV_10b, "Namespace Capacity (NCAP)")            \
    ZZ(IDNAMESPC_NUSE,      16,     8,      true,  SPECREV_10b, "Namespace Utilization (NUSE)")         \
    ZZ(IDNAMESPC_NSFEAT,    24,     1,      true,  SPECREV_10b, "Namespace Features (NSFEAT)")          \
    ZZ(IDNAMESPC_NLBAF,     25,     1,      true,  SPECREV_10b, "Number LBA Formats (NLBAF)")           \
    ZZ(IDNAMESPC_FLBAS,     26,     1,      true,  SPECREV_10b, "Formatted LBA Size (FLBAS)")           \
    ZZ(IDNAMESPC_MC,        27,     1,      true,  SPECREV_10b, "Metadata Capabilities (MC)")           \
    ZZ(IDNAMESPC_DPC,       28,     1,      true,  SPECREV_10b, "End 2 End Data Protection Cap (DPC)")  \
    ZZ(IDNAMESPC_DPS,       29,     1,      true,  SPECREV_10b, "End 2 End Prot Type Settings (DPS)")   \
    ZZ(IDNAMESPC_NMIC,      30,     1,      false, SPECREV_11, "Namspc Multi-path I/O and Namspc Sharing Cap (NMIC)")\
    ZZ(IDNAMESPC_RESCAP,    31,     1,      false, SPECREV_11, "Reservation Capabilities (RESCAP)")    \
    ZZ(IDNAMESPC_RES1E,     30,     98,     true,  SPECREV_10b, "Reserved area @ 0x1e")                 \
    ZZ(IDNAMESPC_RES20,     32,     88,     true,  SPECREV_11, "Reserved area @ 0x20")                 \
    ZZ(IDNAMESPC_FPI,       32,     1,      false, SPECREV_12,  "Format Progress Indicator (FPI)")      \
    ZZ(IDNAMESPC_RES21,     33,     1,      true,  SPECREV_12,  "Reserved area @ 0x21")                 \
    ZZ(IDNAMESPC_NAWUN,     34,     2,      false, SPECREV_12,  "Namespace Atomic Write Unit Normal (NAWUN)")         \
    ZZ(IDNAMESPC_NAWUPF,    36,     2,      false, SPECREV_12,  "Namespace Atomic Write Unit Power Fail (NAWUPF)")    \
    ZZ(IDNAMESPC_NACWU,     38,     2,      false, SPECREV_12,  "Namespace Atomic Compare & Write Unit (NACWU)")      \
    ZZ(IDNAMESPC_NABSN,     40,     2,      false, SPECREV_12,  "Namespace Atomic Boundary Size Normal (NABSN)")      \
    ZZ(IDNAMESPC_NABO,      42,     2,      false, SPECREV_12,  "Namespace Atomic Boundary Offset (NABO)")            \
    ZZ(IDNAMESPC_NABSPF,    44,     2,      false, SPECREV_12,  "Namespace Atomic Boundary Size Power Fail (NABSPF)") \
    ZZ(IDNAMESPC_RES2E,     46,     2,      true,  SPECREV_12,  "Reserved area @ 0x2e")                 \
    ZZ(IDNAMESPC_NVMCAP,    48,     16,     false, SPECREV_12,  "NVM Capacity (NVMCAP)")                \
    ZZ(IDNAMESPC_RES40,     64,     40,     true,  SPECREV_12,  "Reserved area @ 0x40")                 \
    ZZ(IDNAMESPC_NGUID,     104,    16,     false, SPECREV_12,  "Namespace Globally Unique Identifier (NGUID)")       \
    ZZ(IDNAMESPC_EUI64,     120,    8,      true,  SPECREV_11, "IEEE Extended Unique Identifier (EUI64)")            \
    ZZ(IDNAMESPC_LBAF0,     128,    4,      true,  SPECREV_10b, "LBA Format 0 Support (LBAF0)")         \
    ZZ(IDNAMESPC_LBAF1,     132,    4,      false, SPECREV_10b, "LBA Format 1 Support (LBAF1)")         \
    ZZ(IDNAMESPC_LBAF2,     136,    4,      false, SPECREV_10b, "LBA Format 2 Support (LBAF2)")         \
    ZZ(IDNAMESPC_LBAF3,     140,    4,      false, SPECREV_10b, "LBA Format 3 Support (LBAF3)")         \
    ZZ(IDNAMESPC_LBAF4,     144,    4,      false, SPECREV_10b, "LBA Format 4 Support (LBAF4)")         \
    ZZ(IDNAMESPC_LBAF5,     148,    4,      false, SPECREV_10b, "LBA Format 5 Support (LBAF5)")         \
    ZZ(IDNAMESPC_LBAF6,     152,    4,      false, SPECREV_10b, "LBA Format 6 Support (LBAF6)")         \
    ZZ(IDNAMESPC_LBAF7,     156,    4,      false, SPECREV_10b, "LBA Format 7 Support (LBAF7)")         \
    ZZ(IDNAMESPC_LBAF8,     160,    4,      false, SPECREV_10b, "LBA Format 8 Support (LBAF8)")         \
    ZZ(IDNAMESPC_LBAF9,     164,    4,      false, SPECREV_10b, "LBA Format 9 Support (LBAF9)")         \
    ZZ(IDNAMESPC_LBAF10,    168,    4,      false, SPECREV_10b, "LBA Format 10 Support (LBAF10)")       \
    ZZ(IDNAMESPC_LBAF11,    172,    4,      false, SPECREV_10b, "LBA Format 11 Support (LBAF11)")       \
    ZZ(IDNAMESPC_LBAF12,    176,    4,      false, SPECREV_10b, "LBA Format 12 Support (LBAF12)")       \
    ZZ(IDNAMESPC_LBAF13,    180,    4,      false, SPECREV_10b, "LBA Format 13 Support (LBAF13)")       \
    ZZ(IDNAMESPC_LBAF14,    184,    4,      false, SPECREV_10b, "LBA Format 14 Support (LBAF14)")       \
    ZZ(IDNAMESPC_LBAF15,    188,    4,      false, SPECREV_10b, "LBA Format 15 Support (LBAF15)")       \
    ZZ(IDNAMESPC_RESC0,     192,    192,    true,  SPECREV_10b, "Reserved area @ 0xc0")                 \
    ZZ(IDNAMESPC_VS,        384,    3712,   false, SPECREV_10b, "Vendor Specific (VS)")

#define ZZ(a, b, c, d, e, f)         a,
typedef enum IdNamespc
{
    IDNAMESPC_TABLE
    IDNAMESPC_FENCE                // always must be the last element
} IdNamespc;
#undef ZZ

struct LBAFormat {
    uint16_t    MS;
    uint8_t     LBADS;
    uint8_t     RP;
    string parse() {
    	string retStr = "";
    	char   stringToAppend[1024];
    	snprintf(stringToAppend, 1024, " MS   : 0x%04x\n", MS); retStr+=stringToAppend;
    	snprintf(stringToAppend, 1024, " LBADS: 0x%02x\n", LBADS); retStr+=stringToAppend;
    	snprintf(stringToAppend, 1024, " RP   : 0x%02x\n", RP); retStr+=stringToAppend;
    	return retStr;
    }
} __attribute__((__packed__));


struct IdNamespcStruct {
    uint64_t    NSZE;
    uint64_t    NCAP;
    uint64_t    NUSE;
    uint8_t     NSFEAT;
    uint8_t     NLBAF;
    uint8_t     FLBAS;
    uint8_t     MC;
    uint8_t     DPC;
    uint8_t     DPS;
    uint8_t     RES_1E[98];
    struct LBAFormat LBAF[16];
    uint8_t     RES_C0[192];
    uint8_t     VS[3712];

    uint8_t getLBAFormatIndex() {
    	return this->FLBAS & 0x0F;
    }
    LBAFormat getLBAFormat() {
        return this->LBAF[this->getLBAFormatIndex() & 0x0f]; // FLBAS is a 1 byte value. 8 bits. Bottom nibble is the index.
    }

    bool    isMetaDataSeparate() {
    	return ( (this->FLBAS & 0x10) == 0 ); // Bit 4 is 0 separate, else interleaved
    }
    bool    isMetaLast8B() {
    	return ( (this->DPS & 0x08) == 0); // Bit 3 is 0 then last 8B, else first 8B (if e2e enabled)
    }
    bool    isMetaPI() {
    	return ( (this->DPS & 0x07) > 0); // If any bits 0-2 set, then PI is enabled, else disabled
    }

    uint32_t getLBADataSize() {
        return (1 << ( this->getLBAFormat().LBADS) );
    }

    uint32_t getMetaDataSize() {
        return this->getLBAFormat().MS;
    }

    NamspcType getNamespaceType() {
    	if (getMetaDataSize() == 0)
    		return NS_BARE;

        if ( !isMetaPI() ) {
            // Meta namespaces supporting meta data, and E2E is disabled;
            // Implies: Identify.LBAF[Identify.FLBAS].MS=!0, Identify.DPS_b2:0=0
            if ( isMetaDataSeparate() )
                return NS_METAS;
            else
                return NS_METAI;

        } else {
            // Meta namespaces supporting meta data, and E2E is ENABLED;
            // Implies: Identify.LBAF[Identify.FLBAS].MS=!0, Identify.DPS_b2:0=0
            if ( isMetaDataSeparate() )
                return NS_E2ES;
            else
                return NS_E2EI;
        }
        return NS_UNKNOWN;
    }

    string parse() {
    	string retStr = "";
    	char   stringToAppend[1024];
    	snprintf(stringToAppend, 1024, "NSZE  : 0x%016llx\n", (unsigned long long) NSZE); retStr+=stringToAppend;
    	snprintf(stringToAppend, 1024, "NCAP  : 0x%016llx\n", (unsigned long long) NCAP); retStr+=stringToAppend;
    	snprintf(stringToAppend, 1024, "NUSE  : 0x%016llx\n", (unsigned long long) NUSE); retStr+=stringToAppend;
    	snprintf(stringToAppend, 1024, "NSFEAT: 0x%016llx\n", (unsigned long long) NSFEAT); retStr+=stringToAppend;
    	snprintf(stringToAppend, 1024, "NLBAF : 0x%016llx\n", (unsigned long long) NLBAF); retStr+=stringToAppend;
    	snprintf(stringToAppend, 1024, "FLBAS : 0x%016llx\n", (unsigned long long) FLBAS); retStr+=stringToAppend;
    	snprintf(stringToAppend, 1024, "MC    : 0x%02x\n", MC); retStr+=stringToAppend;
    	snprintf(stringToAppend, 1024, "DPC   : 0x%02x\n", DPC); retStr+=stringToAppend;
    	snprintf(stringToAppend, 1024, "DPS   : 0x%02x\n", DPS); retStr+=stringToAppend;
    	for(uint8_t LBAFindex = 0; LBAFindex < 16; LBAFindex++) {
        	snprintf(stringToAppend, 1024, "LBAF[%02d]:\n", LBAFindex); retStr+=stringToAppend;
    		retStr+=LBAF[LBAFindex].parse();
    	}
    	printf("\n%s\n", retStr.c_str() );
    	return retStr;
    }
} __attribute__((__packed__));

struct NamespaceManagementStruct {
    uint64_t    NSZE;
    uint64_t    NCAP;
    uint8_t     Reserved16_25[10];
    uint8_t     FLBAS;
    uint8_t     Reserved27_28[2];
    uint8_t     DPS;
    uint8_t     NMIC;
    uint8_t     Reserved31_383[353];
    string parse() {
    	string retStr = "";
    	char   stringToAppend[1024];
    	snprintf(stringToAppend, 1024, "NSZE  : 0x%016llx\n", (unsigned long long) NSZE); retStr+=stringToAppend;
    	snprintf(stringToAppend, 1024, "NCAP  : 0x%016llx\n", (unsigned long long) NCAP); retStr+=stringToAppend;
    	snprintf(stringToAppend, 1024, "FLBAS : 0x%02x\n", FLBAS); retStr+=stringToAppend;
    	snprintf(stringToAppend, 1024, "DPS   : 0x%02x\n", DPS);   retStr+=stringToAppend;
    	snprintf(stringToAppend, 1024, "NMIC  : 0x%02x\n", NMIC);  retStr+=stringToAppend;
    	printf("\n%s\n", retStr.c_str() );
    	return retStr;
    }
} __attribute__((__packed__));


////////////////////////////////////////////////////////////////////////////////
//                   REGISTER BIT DEFINITIONS FOLLOW
////////////////////////////////////////////////////////////////////////////////

// Figure 86 in 1.2 spec               CNS  NSID CNTID Returns
// CNS_Namespace                      0x00    X     -   (Identify Namespace from NSID X. If attached, receive struct, else all 0's, else invalid namespace ID)
// CNS_Controller                     0x01    -     -   (Identify Controller struct)
// CNS_NamespaceListAttached          0x02    Y     -   (Identify Namespace LIST, starting at NSID Y and in an increasing order)
// CNS_NamespaceListSubsystem         0x10    Z     -   (Identify Namespace LIST, starting at NSID Z present in subsystem that may or may not be attached)
// CNS_NamespaceStructSubsystem       0x11    X     -   (Identify Namespace from NSID A. If attached or not, receive the struct, else invalid namespace ID)
// CNS_ControllerListAttachedToNSID   0x12    X     A   (Controller List that are attached to NSID X, starting with CNTID greater than A)
// CNS_ControllerListSubsystem        0x13    -     B   (Controller List present in subsystem starting with CNTID greater than B)
typedef enum CNSValues {
  	CNS_Namespace                    = 0x00,
  	CNS_Controller                   = 0x01,
  	CNS_NamespaceListAttached        = 0x02,
  	CNS_NamespaceListSubsystem       = 0x10,
  	CNS_NamespaceStructSubsystem     = 0x11,
  	CNS_ControllerListAttachedToNSID = 0x12,
  	CNS_ControllerListSubsystem      = 0x13
} CNSValues;

/// Bit definitions for IDCTRLRCAP_ONCS
typedef enum ONCSBits {
    ONCS_SUP_COMP_CMD           = 0x0001,
    ONCS_SUP_WR_UNC_CMD         = 0x0002,
    ONCS_SUP_DSM_CMD            = 0x0004,
    ONCS_SUP_WR_ZERO_CMD        = 0x0008,          /* 1.1+ only */
    ONCS_SUP_SV_AND_SLCT_FEATS  = 0x0010,          /* 1.1+ only */
    ONCS_SUP_RSRV               = 0x0020           /* 1.1+ only */
} ONCSBits;

/// Bit definitions for IDCTRLRCAP_OACS
typedef enum OACSBits {
    OACS_SUP_SECURITY_CMD     = 0x0001,
    OACS_SUP_FORMAT_NVM_CMD   = 0x0002,
    OACS_SUP_FIRMWARE_CMD     = 0x0004,
    OACS_SUP_NSMANAGEMENT_CMD = 0x0008
} OACSBits;

typedef enum FRMWBits {
    FRMW_SLOT1_RO   = 0x01,
    FRMW_NUM_SLOTS  = 0x0e,
    FRMW_RESV0      = 0xf0
} FRMWBits;

typedef enum RESCAPBits {
    RESCAP_PERSIST          = 0x01,
    RESCAP_WRITE_EXCL       = 0x02,
    RESCAP_EXCL_ACCESS      = 0x04,
    RESCAP_WRITE_EXCL_REG   = 0x08,
    RESCAP_EXCL_ACCESS_REG  = 0x10,
    RESCAP_WRITE_EXCL_ALL   = 0x20,
    RESCAP_EXCL_ACCESS_ALL  = 0x40,
    RESCAP_RESV             = 0x80
} RESCAPBits;

typedef enum NSFEATBits {
    NSFEAT_THIN_PROVISIONING              = 0x01,
    NSFEAT_NAWUN_NAWUPF_NACWU             = 0x02,
    NSFEAT_DEALLOCATED_UNWRITTEN_LBA      = 0x04,
    NSFEAT_RESV                           = 0xF8
} NSFEATBits;

#endif
