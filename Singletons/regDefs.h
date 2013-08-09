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

#ifndef _REGDEFS_H_
#define _REGDEFS_H_

#include <stddef.h>
#include <limits.h>
#include "tnvme.h"

#define MAX_SUPPORTED_REG_SIZE      8       // 64b/8B is max


//------------------------------------------------------------------------------
//----------------------------------PCI-----------------------------------------
//------------------------------------------------------------------------------
typedef enum PciCapabilities {
    PCICAP_PMCAP,
    PCICAP_MSICAP,
    PCICAP_MSIXCAP,
    PCICAP_PXCAP,
    PCICAP_AERCAP,
    PCICAP_FENCE        // always must be the last element
} PciCapabilities;

/* IMPORTANT:
 *   The addition of a new register to the following table is easy, add the new
 * entry and only make its specRev field valid for the newly released
 * specification defining that new register.
 *   The changing of maskRO, impSpec, or dfltValue is not so straight forward.
 * In this case an existing register has changed behavior upon each released
 * specification throughout history and we need to handle each change uniquely.
 * Thus a new entry must be made, mostly duplicating the previous entry, which
 * are only specific to their respective released documents. Thus, the
 * following may occur:
 *
 *  PciSpc,              cap,            offset,  size, specRev,     maskRO,             impSpec,            dfltValue,          desc
 *  ZZ(PCISPC_CC,        PCICAP_FENCE,   0x09,    3,    SPECREV_10b, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, "PCI hdr class codes register (CC)")
 *  ZZ(PCISPC_CC_1,      PCICAP_FENCE,   0x09,    3,    SPECREV_10c, 0xffff00000fffffff, 0x0000000000000000, 0x0000000000000000, "PCI hdr class codes register (CC)")
 *  ZZ(PCISPC_CC_2,      PCICAP_FENCE,   0x09,    3,    SPECREV_10d, 0x0000000000000000, 0x0ffffffffff00000, 0x0000000000000000, "PCI hdr class codes register (CC)")
 */
/*     PciSpc,           cap,            offset,    size, specRev,     maskRO,             impSpec,            dfltValue,          desc */
#define PCISPC_TABLE                                                                                                                                                                   \
    ZZ(PCISPC_ID,        PCICAP_FENCE,   0x00,      4,    SPECREV_10b, 0x00000000ffffffff, 0x00000000ffffffff, 0x0000000000000000, "PCI hdr identifier register (ID)")                 \
    ZZ(PCISPC_CMD,       PCICAP_FENCE,   0x04,      2,    SPECREV_10b, 0x000000000000fab8, 0x0000000000000040, 0x0000000000000000, "PCI hdr cmd register (CMD)")                       \
    ZZ(PCISPC_STS,       PCICAP_FENCE,   0x06,      2,    SPECREV_10b, 0x0000000000004eff, 0x00000000000006a0, 0x0000000000000010, "PCI hdr device status register (STS)")             \
    ZZ(PCISPC_RID,       PCICAP_FENCE,   0x08,      1,    SPECREV_10b, 0x00000000000000ff, 0x00000000000000ff, 0x0000000000000000, "PCI hdr revision ID register (RID)")               \
    ZZ(PCISPC_CC,        PCICAP_FENCE,   0x09,      3,    SPECREV_10b, 0x0000000000ffffff, 0x0000000000000000, 0x0000000000010802, "PCI hdr class codes register (CC)")                \
    ZZ(PCISPC_CLS,       PCICAP_FENCE,   0x0c,      1,    SPECREV_10b, 0x00000000000000ff, 0x0000000000000000, 0x0000000000000000, "PCI hdr cache line size register (CLS)")           \
    ZZ(PCISPC_MLT,       PCICAP_FENCE,   0x0d,      1,    SPECREV_10b, 0x00000000000000ff, 0x0000000000000000, 0x0000000000000000, "PCI hdr master latency timer reg (MLT)")           \
    ZZ(PCISPC_HTYPE,     PCICAP_FENCE,   0x0e,      1,    SPECREV_10b, 0x00000000000000ff, 0x0000000000000080, 0x0000000000000000, "PCI hdr header type register (HTYPE)")             \
    ZZ(PCISPC_BIST,      PCICAP_FENCE,   0x0f,      1,    SPECREV_10b, 0x00000000000000bf, 0x0000000000000080, 0x0000000000000000, "PCI hdr built in self test register (BIST)")       \
    ZZ(PCISPC_BAR0,      PCICAP_FENCE,   0x10,      4,    SPECREV_10b, 0x0000000000003fff, 0x0000000000000000, 0x0000000000000004, "PCI hdr MLBAR register (BAR0)")                    \
    ZZ(PCISPC_BAR1,      PCICAP_FENCE,   0x14,      4,    SPECREV_10b, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, "PCI hdr MUBAR register (BAR1)")                    \
    ZZ(PCISPC_BAR2,      PCICAP_FENCE,   0x18,      4,    SPECREV_10b, 0x0000000000000007, 0x0000000000000000, 0x0000000000000001, "PCI hdr index/data pair register (BAR2)")          \
    ZZ(PCISPC_BAR3,      PCICAP_FENCE,   0x1c,      4,    SPECREV_10b, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, "PCI hdr reserved area (BAR3)")                     \
    ZZ(PCISPC_BAR4,      PCICAP_FENCE,   0x20,      4,    SPECREV_10b, 0x0000000000000000, 0x00000000ffffffff, 0x0000000000000000, "PCI hdr vendor specific register (BAR4)")          \
    ZZ(PCISPC_BAR5,      PCICAP_FENCE,   0x24,      4,    SPECREV_10b, 0x0000000000000000, 0x00000000ffffffff, 0x0000000000000000, "PCI hdr vendor specific register (BAR5)")          \
    ZZ(PCISPC_CCPTR,     PCICAP_FENCE,   0x28,      4,    SPECREV_10b, 0x00000000ffffffff, 0x0000000000000000, 0x0000000000000000, "PCI hdr CardBus CIS register (CCPTR)")             \
    ZZ(PCISPC_SS,        PCICAP_FENCE,   0x2c,      4,    SPECREV_10b, 0x00000000ffffffff, 0x00000000ffffffff, 0x0000000000000000, "PCI hdr subsystem ID register (SS)")               \
    ZZ(PCISPC_EROM,      PCICAP_FENCE,   0x30,      4,    SPECREV_10b, 0x0000000000000000, 0x00000000ffffffff, 0x0000000000000000, "PCI hdr expansion ROM register (EROM)")            \
    ZZ(PCISPC_CAP,       PCICAP_FENCE,   0x34,      1,    SPECREV_10b, 0x00000000000000ff, 0x00000000000000ff, 0x0000000000000000, "PCI hdr CAP ptr register (CAP)")                   \
    ZZ(PCISPC_RES0,      PCICAP_FENCE,   0x35,      7,    SPECREV_10b, 0x00ffffffffffffff, 0x0000000000000000, 0x0000000000000000, "PCI hdr reserved area #0")                         \
    ZZ(PCISPC_INTR,      PCICAP_FENCE,   0x3c,      2,    SPECREV_10b, 0x000000000000ff00, 0x000000000000ff00, 0x0000000000000000, "PCI hdr interrupt register (INTR)")                \
    ZZ(PCISPC_MGNT,      PCICAP_FENCE,   0x3e,      1,    SPECREV_10b, 0x00000000000000ff, 0x0000000000000000, 0x0000000000000000, "PCI hdr min grant register (MGMT)")                \
    ZZ(PCISPC_MLAT,      PCICAP_FENCE,   0x3f,      1,    SPECREV_10b, 0x00000000000000ff, 0x0000000000000000, 0x0000000000000000, "PCI hdr max latency register (MLAT)")              \
    ZZ(PCISPC_PID,       PCICAP_PMCAP,   USHRT_MAX, 2,    SPECREV_10b, 0x000000000000ffff, 0x000000000000fffe, 0x0000000000000001, "PCI power mgmt ID register (PID)")                 \
    ZZ(PCISPC_PC,        PCICAP_PMCAP,   USHRT_MAX, 2,    SPECREV_10b, 0x000000000000ffff, 0x0000000000000027, 0x0000000000000000, "PCI power mgmt CAP register (PC)")                 \
    ZZ(PCISPC_PMCS,      PCICAP_PMCAP,   USHRT_MAX, 2,    SPECREV_10b, 0x000000000000fffc, 0x0000000000000000, 0x0000000000000008, "PCI pwr mgmt ctrl & status reg (PMCS)")            \
    ZZ(PCISPC_MID,       PCICAP_MSICAP,  USHRT_MAX, 2,    SPECREV_10b, 0x000000000000ffff, 0x000000000000ff00, 0x0000000000000005, "PCI MSI ID register (MID)")                        \
    ZZ(PCISPC_MC,        PCICAP_MSICAP,  USHRT_MAX, 2,    SPECREV_10b, 0x000000000000ff8e, 0x000000000000000e, 0x0000000000000080, "PCI MSI msg ctrl register (MC)")                   \
    ZZ(PCISPC_MA,        PCICAP_MSICAP,  USHRT_MAX, 4,    SPECREV_10b, 0x0000000000000003, 0x0000000000000000, 0x0000000000000000, "PCI MSI msg addr  register (MA)")                  \
    ZZ(PCISPC_MUA,       PCICAP_MSICAP,  USHRT_MAX, 4,    SPECREV_10b, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, "PCI MSI msg upper addr register (MUA)")            \
    ZZ(PCISPC_MD,        PCICAP_MSICAP,  USHRT_MAX, 2,    SPECREV_10b, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, "PCI MSI msg data register (MD)")                   \
    ZZ(PCISPC_MMASK,     PCICAP_MSICAP,  USHRT_MAX, 4,    SPECREV_10b, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, "PCI MSI mask bits register (MMASK)")               \
    ZZ(PCISPC_MPEND,     PCICAP_MSICAP,  USHRT_MAX, 4,    SPECREV_10b, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, "PCI MSI pending bits register (MPEND)")            \
    ZZ(PCISPC_MXID,      PCICAP_MSIXCAP, USHRT_MAX, 2,    SPECREV_10b, 0x000000000000ffff, 0x000000000000ff00, 0x0000000000000011, "PCI MSI-X ID register (MXID)")                     \
    ZZ(PCISPC_MXC,       PCICAP_MSIXCAP, USHRT_MAX, 2,    SPECREV_10b, 0x0000000000003fff, 0x00000000000007ff, 0x0000000000000000, "PCI MSI-X msg ctrl register (MXC)")                \
    ZZ(PCISPC_MTAB,      PCICAP_MSIXCAP, USHRT_MAX, 4,    SPECREV_10b, 0x00000000ffffffff, 0x00000000ffffffff, 0x0000000000000000, "PCI MSI-X table offset/BIR register (MTAB)")       \
    ZZ(PCISPC_MPBA,      PCICAP_MSIXCAP, USHRT_MAX, 4,    SPECREV_10b, 0x00000000ffffffff, 0x00000000ffffffff, 0x0000000000000000, "PCI MSI-X PBA offset/BIR register (MPBA)")         \
    ZZ(PCISPC_PXID,      PCICAP_PXCAP,   USHRT_MAX, 2,    SPECREV_10b, 0x000000000000ffff, 0x000000000000ff00, 0x0000000000000010, "PCI express CAP ID register (PXID)")               \
    ZZ(PCISPC_PXCAP,     PCICAP_PXCAP,   USHRT_MAX, 2,    SPECREV_10b, 0x000000000000ffff, 0x0000000000003e00, 0x0000000000000002, "PCI express CAP register (PXCAP)")                 \
    ZZ(PCISPC_PXDCAP,    PCICAP_PXCAP,   USHRT_MAX, 4,    SPECREV_10b, 0x00000000ffffffff, 0x0000000000000fff, 0x0000000010008000, "PCI express device CAP register (PXDCAP)")         \
    ZZ(PCISPC_PXDC,      PCICAP_PXCAP,   USHRT_MAX, 2,    SPECREV_10b, 0x0000000000000000, 0x000000000000fff0, 0x0000000000000000, "PCI express device ctrl register (PXDC)")          \
    ZZ(PCISPC_PXDS,      PCICAP_PXCAP,   USHRT_MAX, 2,    SPECREV_10b, 0x000000000000fff0, 0x0000000000000010, 0x0000000000000000, "PCI express device status register (PXDS)")        \
    ZZ(PCISPC_PXLCAP,    PCICAP_PXCAP,   USHRT_MAX, 4,    SPECREV_10b, 0x00000000ffffffff, 0x00000000ff07ffff, 0x0000000000000000, "PCI express link CAP register (PXLCAP)")           \
    ZZ(PCISPC_PXLC,      PCICAP_PXCAP,   USHRT_MAX, 2,    SPECREV_10b, 0x000000000000fc34, 0x0000000000000200, 0x0000000000000000, "PCI express link ctrl register (PXLC)")            \
    ZZ(PCISPC_PXLS,      PCICAP_PXCAP,   USHRT_MAX, 2,    SPECREV_10b, 0x000000000000ffff, 0x00000000000013ff, 0x0000000000000000, "PCI express link status register (PXLS)")          \
    ZZ(PCISPC_RES1,      PCICAP_PXCAP,   USHRT_MAX, 16,   SPECREV_10b, 0xffffffffffffffff, 0x0000000000000000, 0x0000000000000000, "PCI hdr reserved area #1")                         \
    ZZ(PCISPC_PXDCAP2,   PCICAP_PXCAP,   USHRT_MAX, 4,    SPECREV_10b, 0x00000000ffffffff, 0x0000000000fc3b8f, 0x0000000000000010, "PCI express device CAP2 register (PXDCAP2)")       \
    ZZ(PCISPC_PXDC2,     PCICAP_PXCAP,   USHRT_MAX, 2,    SPECREV_10b, 0x00000000ffff9b0f, 0x000000000000700f, 0x0000000000000000, "PCI express device ctrl2 register (PXDC2)")        \
    ZZ(PCISPC_AERID,     PCICAP_AERCAP,  USHRT_MAX, 4,    SPECREV_10b, 0x00000000ffffffff, 0x00000000fff00000, 0x0000000000020001, "PCI AER CAP ID register (AERID)")                  \
    ZZ(PCISPC_AERUCES,   PCICAP_AERCAP,  USHRT_MAX, 4,    SPECREV_10b, 0x00000000fc0007ef, 0x00000000003fff0f, 0x0000000000000000, "PCI AER uncorrect err stat register (AERECES)")    \
    ZZ(PCISPC_AERUCEM,   PCICAP_AERCAP,  USHRT_MAX, 4,    SPECREV_10b, 0x00000000fc000fef, 0x000000000000f1c1, 0x0000000000400000, "PCI AER uncorrect err mask register (AERUCEM)")    \
    ZZ(PCISPC_AERUCESEV, PCICAP_AERCAP,  USHRT_MAX, 4,    SPECREV_10b, 0x00000000fc000fef, 0x0000000003eaa000, 0x0000000000462010, "PCI AER uncorrect err severity reg (AERECESEV)")   \
    ZZ(PCISPC_AERCS,     PCICAP_AERCAP,  USHRT_MAX, 4,    SPECREV_10b, 0x00000000ffff0e3e, 0x000000000000c000, 0x0000000000000000, "PCI AER correctable err stat reg (AERCES)")        \
    ZZ(PCISPC_AERCEM,    PCICAP_AERCAP,  USHRT_MAX, 4,    SPECREV_10b, 0x00000000ffff0e3e, 0x000000000000c000, 0x0000000000000000, "PCI AER correctable err mask reg (AERCEM)")        \
    ZZ(PCISPC_AERCC,     PCICAP_AERCAP,  USHRT_MAX, 4,    SPECREV_10b, 0x00000000fffff8af, 0x00000000000007e0, 0x0000000000000000, "PCI AER adv err CAP & ctrl register AERCC)")       \
    ZZ(PCISPC_AERHL,     PCICAP_AERCAP,  USHRT_MAX, 16,   SPECREV_10b, 0xffffffffffffffff, 0x0000000000000000, 0x0000000000000000, "PCI AER header log register (AERHL)")              \
    ZZ(PCISPC_AERTLP,    PCICAP_AERCAP,  USHRT_MAX, 16,   SPECREV_10b, 0xffffffffffffffff, 0x0000000000000000, 0x0000000000000000, "PCI AER TLP prefix log register (AERTLP)")

#define ZZ(a, b, c, d, e, f, g, h, i)      a,
typedef enum PciSpc
{
    PCISPC_TABLE
    PCISPC_FENCE                // always must be the last element
} PciSpc;
#undef ZZ

struct PciSpcType {
    PciCapabilities cap;
    uint16_t        offset;     // ==USHRT_MAX implies offset is unknown
    uint16_t        size;
    SpecRev         specRev;
    uint64_t        maskRO;
    uint64_t        impSpec;    // implementation specific
    uint64_t        dfltValue;
    const char     *desc;
};


//------------------------------------------------------------------------------
//----------------------------------CTRL'R--------------------------------------
//------------------------------------------------------------------------------
/* IMPORTANT:
 *   The addition of a new register to the following table is easy, add the new
 * entry and only make its specRev field valid for the newly released
 * specification defining that new register.
 *   The changing of maskRO, impSpec, or dfltValue is not so straight forward.
 * In this case an existing register has changed behavior upon each released
 * specification throughout history and we need to handle each change uniquely.
 * Thus a new entry must be made, mostly duplicating the previous entry, which
 * are only specific to their respective released documents. Thus, the
 * following may occur:
 *
 *  CtlSpc,              offset,  size, specRev,     maskRO,             impSpec,            dfltValue,          desc
 *  ZZ(CTLSPC_CC,        0x14,    4,    SPECREV_10b, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, "ctrlr config register (CC)")
 *  ZZ(CTLSPC_CC_1,      0x14,    4,    SPECREV_10c, 0x00fffffffff00000, 0x0000000000000000, 0x0000000000000000, "ctrlr config register (CC)")
 *  ZZ(CTLSPC_CC_2,      0x14,    4,    SPECREV_10d, 0x0000000000000000, 0x0000000000000000, 0x0ffffffffff00000, "ctrlr config register (CC)")
 */
/*     CtlSpc,           offset,  size, specRev,     maskRO,             impSpec,            dfltValue,          desc */
#define CTLSPC_TABLE                                                                                                                                             \
    ZZ(CTLSPC_CAP,       0x00,    8,    SPECREV_10b, 0xffffffffffffffff, 0x00ff01efff07ffff, 0x0000000000000000, "ctrlr CAP register (CAP)")                     \
    ZZ(CTLSPC_VS,        0x08,    4,    SPECREV_10b, 0x00000000ffffffff, 0x0000000000000000, 0x0000000000010000, "ctrlr version register (VS)")                  \
    ZZ(CTLSPC_INTMS,     0x0c,    4,    SPECREV_10b, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, "ctrlr interrupt mask set register (INTMS)")    \
    ZZ(CTLSPC_INTMC,     0x10,    4,    SPECREV_10b, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, "ctrlr interrupt mask clear register (INTMC)")  \
    ZZ(CTLSPC_CC,        0x14,    4,    SPECREV_10b, 0x00000000ff00000e, 0x0000000000000000, 0x0000000000000000, "ctrlr config register (CC)")                   \
    ZZ(CTLSPC_RES0,      0x18,    4,    SPECREV_10b, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, "ctrlr reserved area 0x18")                     \
    ZZ(CTLSPC_CSTS,      0x1c,    4,    SPECREV_10b, 0x00000000ffffffff, 0x0000000000000000, 0x0000000000000000, "ctrlr status register (CSTS)")                 \
    ZZ(CTLSPC_RES1,      0x20,    4,    SPECREV_10b, 0x00000000ffffffff, 0x0000000000000000, 0x0000000000000000, "ctrlr reserved area 0x20")                     \
    ZZ(CTLSPC_AQA,       0x24,    4,    SPECREV_10b, 0x00000000f000f000, 0x0000000000000000, 0x0000000000000000, "ctrlr admin Q attrib register (AQA)")          \
    ZZ(CTLSPC_ASQ,       0x28,    8,    SPECREV_10b, 0x0000000000000fff, 0x00000000fffff000, 0x0000000000000000, "ctrlr admin SQ BAR register (ASQ)")            \
    ZZ(CTLSPC_ACQ,       0x30,    8,    SPECREV_10b, 0x0000000000000fff, 0x00000000fffff000, 0x0000000000000000, "ctrlr admin CQ BAR register (ACQ)")            \
    ZZ(CTLSPC_RES2,      0x38,    3784, SPECREV_10b, 0xffffffffffffffff, 0x0000000000000000, 0x0000000000000000, "ctrlr reserved area 0x38")                     \
    ZZ(CTLSPC_RES3,      0xf00,   256,  SPECREV_10b, 0xffffffffffffffff, 0x0000000000000000, 0x0000000000000000, "ctrlr cmd set specific (reserved)")
    // NOTE: doorbell registers are to variable/numerous to handle in this manner.

#define ZZ(a, b, c, d, e, f, g, h)         a,
typedef enum CtlSpc
{
    CTLSPC_TABLE
    CTLSPC_FENCE                // always must be the last element
} CtlSpc;
#undef ZZ

struct CtlSpcType {
    uint16_t        offset;     // ==USHRT_MAX implies offset is unknown
    uint16_t        size;
    SpecRev         specRev;
    uint64_t        maskRO;
    uint64_t        impSpec;    // implementation specific
    uint64_t        dfltValue;
    const char     *desc;
};


////////////////////////////////////////////////////////////////////////////////
//                   REGISTER BIT DEFINITIONS FOLLOW
////////////////////////////////////////////////////////////////////////////////

/// Bit definitions for PCISPC_CMD
typedef enum CMDBits {
    CMD_IOSE       = 0x0001,
    CMD_MSE        = 0x0002,
    CMD_BME        = 0x0004,
    CMD_SCE        = 0x0008
} CMDBits;


/// Bit definitions for PCISPC_STS
typedef enum STSBits {
    STS_RES0       = 0x0007,
    STS_IS         = 0x0008,
    STS_CL         = 0x0010,
    STS_C66        = 0x0020,
    STS_RES1       = 0x0040,
    STS_FBC        = 0x0080,
    STS_DPD        = 0x0100,
    STS_DEVT       = 0x0600,
    STS_STA        = 0x0800,
    STS_RTA        = 0x1000,
    STS_RMA        = 0x2000,
    STS_SSE        = 0x4000,
    STS_DPE        = 0x8000
} STSBits;


/// Bit definitions for PCISPC_PXDS
typedef enum PXDSBits {
    PXDS_CED       = 0x0001,
    PXDS_NFED      = 0x0002,
    PXDS_FED       = 0x0004,
    PXDS_URD       = 0x0008,
    PXDS_APD       = 0x0010,
    PXDS_TP        = 0x0040
} PXDSBits;


/// Bit definitions for PCISPC_MC
typedef enum MCBits {
    MC_RES0        = 0xfe00,
    MC_PVM         = 0x0100,
    MC_C64         = 0x0080,
    MC_MME         = 0x0070,
    MC_MMC         = 0x000e,
    MC_MSIE        = 0x0001
} MCBits;


/// Bit definitions for PCISPC_MXC
typedef enum MXCBits {
    MXC_MXE         = 0x8000,
    MXC_FM          = 0x4000,
    MXC_RES0        = 0x3800,
    MXC_TS          = 0x07ff
} MXCBits;


/// Bit definitions for PCISPC_AERUCES
typedef enum AERUCESBits {
    AERUCES_RES0   = 0x0000000f,
    AERUCES_DLPES  = 0x00000010,
    AERUCES_RES1   = 0x00000fe0,
    AERUCES_PTS    = 0x00001000,
    AERUCES_FCPES  = 0x00002000,
    AERUCES_CTS    = 0x00004000,
    AERUCES_CAS    = 0x00008000,
    AERUCES_UCS    = 0x00010000,
    AERUCES_ROS    = 0x00020000,
    AERUCES_MTS    = 0x00040000,
    AERUCES_ECRCES = 0x00080000,
    AERUCES_URES   = 0x00100000,
    AERUCES_ACSVS  = 0x00200000,
    AERUCES_UIES   = 0x00400000,
    AERUCES_MCBTS  = 0x00800000,
    AERUCES_AOEBS  = 0x01000000,
    AERUCES_TPBES  = 0x02000000,
    AERUCES_RES2   = 0xfc000000
} AERUCESBits;


/// Bit definitions for CTLSPC_CSTS
typedef enum CSTSBits {
    CSTS_RES0      = 0xfffffff0,
    CSTS_SHST      = 0x0000000c,
    CSTS_CFS       = 0x00000002,
    CSTS_RDY       = 0x00000001
} CSTSBits;


/// Bit definitions for CTLSPC_CC
typedef enum CCBits {
    CC_RES0        = 0xff000000,
    CC_IOCQES      = 0x00f00000,
    CC_IOSQES      = 0x000f0000,
    CC_SHN         = 0x0000C000,
    CC_AMS         = 0x00003800,
    CC_MPS         = 0x00000780,
    CC_CSS         = 0x00000070,
    CC_RES1        = 0x0000000e,
    CC_EN          = 0000000001
} CCBits;


/// Bit definitions for CTLSPC_CAP
typedef enum CAPBits {
    CAP_RES0       = 0xff00000000000000,
    CAP_MPSMAX     = 0x00f0000000000000,
    CAP_MPSMIN     = 0x000f000000000000,
    CAP_RES1       = 0x0000fe0000000000,
    CAP_CSS        = 0x000001e000000000,
    CAP_RES2       = 0x0000001000000000,
    CAP_DSTRD      = 0x0000000f00000000,
    CAP_TO         = 0x00000000ff000000,
    CAP_RES3       = 0x0000000000f80000,
    CAP_AMS        = 0x0000000000060000,
    CAP_CQR        = 0x0000000000010000,
    CAP_MQES       = 0x000000000000ffff
} CAPBits;


#endif
