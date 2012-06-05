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
    const char *desc;          // this fields formal description
};

/*     IdCtrlrCap,          offset, length, desc                                   */
#define IDCTRLRCAP_TABLE                                                            \
    ZZ(IDCTRLRCAP_VID,      0,      2,      "PCI Vendor ID (VID)")                  \
    ZZ(IDCTRLRCAP_SSVID,    2,      2,      "PCI Subsystem Vendor ID (SSVID)")      \
    ZZ(IDCTRLRCAP_SN,       4,      20,     "Serial Number (SN)")                   \
    ZZ(IDCTRLRCAP_MN,       24,     40,     "Model Number (MN)")                    \
    ZZ(IDCTRLRCAP_FR,       64,     8,      "Firmware Revision (FR)")               \
    ZZ(IDCTRLRCAP_RAB,      72,     1,      "Recommend Arbitration Burst (RAB)")    \
    ZZ(IDCTRLRCAP_IEEE,     73,     3,      "IEEE OUI Identifier (IEEE)")           \
    ZZ(IDCTRLRCAP_MIC,      76,     1,      "Multi-Interface Capabilities (MIC)")   \
    ZZ(IDCTRLRCAP_MDTS,     77,     1,      "Maximum Data Xfer Size (MDTS)")        \
    ZZ(IDCTRLRCAP_RES4E,    78,     178,    "Reserved area @ 0x4e")                 \
    ZZ(IDCTRLRCAP_OACS,     256,    2,      "Optional Admin Cmd Support (OACS)")    \
    ZZ(IDCTRLRCAP_ACL,      258,    1,      "Abort Cmd Limit (ACL)")                \
    ZZ(IDCTRLRCAP_AERL,     259,    1,      "Async Event Req Limit (AERL)")         \
    ZZ(IDCTRLRCAP_FRMW,     260,    1,      "Firmware Updates (FRMW)")              \
    ZZ(IDCTRLRCAP_LPA,      261,    1,      "Log Page Atribute (LPA)")              \
    ZZ(IDCTRLRCAP_ELPE,     262,    1,      "Error Log Page Entries (ELPE)")        \
    ZZ(IDCTRLRCAP_NPSS,     263,    1,      "Number of Power States Support (NPSS)")\
    ZZ(IDCTRLRCAP_RES108,   264,    248,    "Reserved area @ 0x108")                \
    ZZ(IDCTRLRCAP_SQES,     512,    1,      "Submission Q Entry Size (SQES)")       \
    ZZ(IDCTRLRCAP_CQES,     513,    1,      "Completion Q Entry Size (CQES)")       \
    ZZ(IDCTRLRCAP_RES202,   514,    2,      "Reserved area @ 0x202")                \
    ZZ(IDCTRLRCAP_NN,       516,    4,      "Number of Namespaces (NN)")            \
    ZZ(IDCTRLRCAP_ONCS,     520,    2,      "Optional NVM Cmd Support (ONCS)")      \
    ZZ(IDCTRLRCAP_FUSES,    522,    2,      "Fused Operation Suppot (FUSES)")       \
    ZZ(IDCTRLRCAP_FNA,      524,    1,      "Format NVM Attribute (FNA)")           \
    ZZ(IDCTRLRCAP_VWC,      525,    1,      "Volatile Write Cache (VWC)")           \
    ZZ(IDCTRLRCAP_AWUN,     526,    2,      "Atomic Write Unit Normal (AWUN)")      \
    ZZ(IDCTRLRCAP_AWUPF,    528,    2,      "Atomic Write Unit Power Fail (AWUPF)") \
    ZZ(IDCTRLRCAP_RES212,   530,    174,    "Reserved area @ 0x212")                \
    ZZ(IDCTRLRCAP_RES2C0,   704,    1344,   "Reserved area @ 2c0")                  \
    ZZ(IDCTRLRCAP_PSD0,     2048,   32,     "Power State 0 Desc (PSD0)")            \
    ZZ(IDCTRLRCAP_PSD1,     2080,   32,     "Power State 1 Desc (PSD1)")            \
    ZZ(IDCTRLRCAP_PSD2,     2112,   32,     "Power State 2 Desc (PSD2)")            \
    ZZ(IDCTRLRCAP_PSD3,     2144,   32,     "Power State 3 Desc (PSD3)")            \
    ZZ(IDCTRLRCAP_PSD4,     2176,   32,     "Power State 4 Desc (PSD4)")            \
    ZZ(IDCTRLRCAP_PSD5,     2208,   32,     "Power State 5 Desc (PSD5)")            \
    ZZ(IDCTRLRCAP_PSD6,     2240,   32,     "Power State 6 Desc (PSD6)")            \
    ZZ(IDCTRLRCAP_PSD7,     2272,   32,     "Power State 7 Desc (PSD7)")            \
    ZZ(IDCTRLRCAP_PSD8,     2304,   32,     "Power State 8 Desc (PSD8)")            \
    ZZ(IDCTRLRCAP_PSD9,     2336,   32,     "Power State 9 Desc (PSD9)")            \
    ZZ(IDCTRLRCAP_PSD10,    2368,   32,     "Power State 10 Desc (PSD10)")          \
    ZZ(IDCTRLRCAP_PSD11,    2400,   32,     "Power State 11 Desc (PSD11)")          \
    ZZ(IDCTRLRCAP_PSD12,    2432,   32,     "Power State 12 Desc (PSD12)")          \
    ZZ(IDCTRLRCAP_PSD13,    2464,   32,     "Power State 13 Desc (PSD13)")          \
    ZZ(IDCTRLRCAP_PSD14,    2496,   32,     "Power State 14 Desc (PSD14)")          \
    ZZ(IDCTRLRCAP_PSD15,    2528,   32,     "Power State 15 Desc (PSD15)")          \
    ZZ(IDCTRLRCAP_PSD16,    2560,   32,     "Power State 16 Desc (PSD16)")          \
    ZZ(IDCTRLRCAP_PSD17,    2592,   32,     "Power State 17 Desc (PSD17)")          \
    ZZ(IDCTRLRCAP_PSD18,    2624,   32,     "Power State 18 Desc (PSD18)")          \
    ZZ(IDCTRLRCAP_PSD19,    2656,   32,     "Power State 19 Desc (PSD19)")          \
    ZZ(IDCTRLRCAP_PSD20,    2688,   32,     "Power State 20 Desc (PSD20)")          \
    ZZ(IDCTRLRCAP_PSD21,    2720,   32,     "Power State 21 Desc (PSD21)")          \
    ZZ(IDCTRLRCAP_PSD22,    2752,   32,     "Power State 22 Desc (PSD22)")          \
    ZZ(IDCTRLRCAP_PSD23,    2784,   32,     "Power State 23 Desc (PSD23)")          \
    ZZ(IDCTRLRCAP_PSD24,    2816,   32,     "Power State 24 Desc (PSD24)")          \
    ZZ(IDCTRLRCAP_PSD25,    2848,   32,     "Power State 25 Desc (PSD25)")          \
    ZZ(IDCTRLRCAP_PSD26,    2880,   32,     "Power State 26 Desc (PSD26)")          \
    ZZ(IDCTRLRCAP_PSD27,    2912,   32,     "Power State 27 Desc (PSD27)")          \
    ZZ(IDCTRLRCAP_PSD28,    2944,   32,     "Power State 28 Desc (PSD28)")          \
    ZZ(IDCTRLRCAP_PSD29,    2976,   32,     "Power State 29 Desc (PSD29)")          \
    ZZ(IDCTRLRCAP_PSD30,    3008,   32,     "Power State 30 Desc (PSD30)")          \
    ZZ(IDCTRLRCAP_PSD31,    3040,   32,     "Power State 31 Desc (PSD31)")          \
    ZZ(IDCTRLRCAP_VS,       3072,   1024,   "Vendor Specific (VS)")

#define ZZ(a, b, c, d)         a,
typedef enum IdCtrlrCap
{
    IDCTRLRCAP_TABLE
    IDCTRLRCAP_FENCE               // always must be the last element
} IdCtrlrCap;
#undef ZZ

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

struct IdCtrlrCapStruct {
    uint16_t    VID;
    uint16_t    SSVID;
    uint8_t     SN[20];
    uint8_t     MN[40];
    uint8_t     FR[8];
    uint8_t     RAB;
    uint8_t     IEEE[3];
    uint8_t     MIC;
    uint8_t     MDTS;
    uint8_t     RES_4E[178];
    uint16_t    OACS;
    uint8_t     ACL;
    uint8_t     AERL;
    uint8_t     FRMW;
    uint8_t     LPA;
    uint8_t     ELPE;
    uint8_t     NPSS;
    uint8_t     RES_108[248];
    uint8_t     SQES;
    uint8_t     CQES;
    uint16_t    RES_202;
    uint32_t    NN;
    uint16_t    ONCS;
    uint16_t    FUSES;
    uint8_t     FNA;
    uint8_t     VWC;
    uint16_t    AWUN;
    uint16_t    SWUP;
    uint8_t     RES_212[174];
    uint8_t     RES_2C0[1344];
    struct IdPowerStateDesc PSD[32];
    uint8_t     VS[1024];
} __attribute__((__packed__));


/*     IdNamespc,           offset, length, desc                                   */
#define IDNAMESPC_TABLE                                                             \
    ZZ(IDNAMESPC_NSZE,      0,      8,      "Namespace Size (NSZE)")                \
    ZZ(IDNAMESPC_NCAP,      8,      8,      "Namespace Capacity (NCAP)")            \
    ZZ(IDNAMESPC_NUSE,      16,     8,      "Namespace Utilization (NUSE)")         \
    ZZ(IDNAMESPC_NSFEAT,    24,     1,      "Namespace Features (NSFEAT)")          \
    ZZ(IDNAMESPC_NLBAF,     25,     1,      "Number LBA Formats (NLBAF)")           \
    ZZ(IDNAMESPC_FLBAS,     26,     1,      "Formatted LBA Size (FLBAS)")           \
    ZZ(IDNAMESPC_MC,        27,     1,      "Metadata Capabilities (MC)")           \
    ZZ(IDNAMESPC_DPC,       28,     1,      "End 2 End Data Protection Cap (DPC)")  \
    ZZ(IDNAMESPC_DPS,       29,     1,      "End 2 End Prot Type Settings (DPS)")   \
    ZZ(IDNAMESPC_RES1E,     30,     98,     "Reserved area @ 0x1e")                 \
    ZZ(IDNAMESPC_LBAF0,     128,    4,      "LBA Format 0 Support (LBAF0)")         \
    ZZ(IDNAMESPC_LBAF1,     132,    4,      "LBA Format 1 Support (LBAF1)")         \
    ZZ(IDNAMESPC_LBAF2,     136,    4,      "LBA Format 2 Support (LBAF2)")         \
    ZZ(IDNAMESPC_LBAF3,     140,    4,      "LBA Format 3 Support (LBAF3)")         \
    ZZ(IDNAMESPC_LBAF4,     144,    4,      "LBA Format 4 Support (LBAF4)")         \
    ZZ(IDNAMESPC_LBAF5,     148,    4,      "LBA Format 5 Support (LBAF5)")         \
    ZZ(IDNAMESPC_LBAF6,     152,    4,      "LBA Format 6 Support (LBAF6)")         \
    ZZ(IDNAMESPC_LBAF7,     156,    4,      "LBA Format 7 Support (LBAF7)")         \
    ZZ(IDNAMESPC_LBAF8,     160,    4,      "LBA Format 8 Support (LBAF8)")         \
    ZZ(IDNAMESPC_LBAF9,     164,    4,      "LBA Format 9 Support (LBAF9)")         \
    ZZ(IDNAMESPC_LBAF10,    168,    4,      "LBA Format 10 Support (LBAF10)")       \
    ZZ(IDNAMESPC_LBAF11,    172,    4,      "LBA Format 11 Support (LBAF11)")       \
    ZZ(IDNAMESPC_LBAF12,    176,    4,      "LBA Format 12 Support (LBAF12)")       \
    ZZ(IDNAMESPC_LBAF13,    180,    4,      "LBA Format 13 Support (LBAF13)")       \
    ZZ(IDNAMESPC_LBAF14,    184,    4,      "LBA Format 14 Support (LBAF14)")       \
    ZZ(IDNAMESPC_LBAF15,    188,    4,      "LBA Format 15 Support (LBAF15)")       \
    ZZ(IDNAMESPC_RESC0,     192,    192,    "Reserved area @ 0xc0")                 \
    ZZ(IDNAMESPC_VS,        384,    3712,   "Vendor Specific (VS)")

#define ZZ(a, b, c, d)         a,
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
} __attribute__((__packed__));


////////////////////////////////////////////////////////////////////////////////
//                   REGISTER BIT DEFINITIONS FOLLOW
////////////////////////////////////////////////////////////////////////////////

/// Bit definitions for IDCTRLRCAP_ONCS
typedef enum ONCSBits {
    ONCS_SUP_COMP_CMD    = 0x0001,
    ONCS_SUP_WR_UNC_CMD  = 0x0002,
    ONCS_SUP_DSM_CMD     = 0x0004
} ONCSBits;

/// Bit definitions for IDCTRLRCAP_OACS
typedef enum OACSBits {
    OACS_SUP_SECURITY_CMD     = 0x0001,
    OACS_SUP_FORMAT_NVM_CMD   = 0x0002,
    OACS_SUP_FIRMWARE_CMD     = 0x0004
} OACSBits;

#endif
