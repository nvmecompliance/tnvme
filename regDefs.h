#ifndef _REGDEFS_H_
#define _REGDEFS_H_

#include <stddef.h>
#include <limits.h>

#define MAX_SUPPORTED_REG_SIZE      8       // 64b/8B is max


//------------------------------------------------------------------------------
typedef enum PciCapabilities {
    PCICAP_PMCAP,
    PCICAP_MSICAP,
    PCICAP_MSIXCAP,
    PCICAP_PXCAP,
    PCICAP_AERCAP,
    PCICAP_FENCE        // always must be the last element
} PciCapabilities;

/*    PciSpc,            cap,            offset,  size, desc */
#define PCISPC_TABLE                                                                                        \
    ZZ(PCISPC_ID,        PCICAP_FENCE,   0x00,    4,    "PCI hdr identifier register (ID)")                 \
    ZZ(PCISPC_CMD,       PCICAP_FENCE,   0x04,    2,    "PCI hdr cmd register (CMD)")                       \
    ZZ(PCISPC_STS,       PCICAP_FENCE,   0x06,    2,    "PCI hdr device status register (STS)")             \
    ZZ(PCISPC_RID,       PCICAP_FENCE,   0x08,    1,    "PCI hdr revision ID register (RID)")               \
    ZZ(PCISPC_CC,        PCICAP_FENCE,   0x09,    3,    "PCI hdr class codes register (CC)")                \
    ZZ(PCISPC_CLS,       PCICAP_FENCE,   0x0c,    1,    "PCI hdr cache line size register (CLS)")           \
    ZZ(PCISPC_MLT,       PCICAP_FENCE,   0x0d,    1,    "PCI hdr master latency timer reg (MLT)")           \
    ZZ(PCISPC_HTYPE,     PCICAP_FENCE,   0x0e,    1,    "PCI hdr header type register (HTYPE)")             \
    ZZ(PCISPC_BIST,      PCICAP_FENCE,   0x0f,    1,    "PCI hdr built in self test register (BIST)")       \
    ZZ(PCISPC_BAR0,      PCICAP_FENCE,   0x10,    4,    "PCI hdr MLBAR register (BAR0)")                    \
    ZZ(PCISPC_BAR1,      PCICAP_FENCE,   0x14,    4,    "PCI hdr MUBAR register (BAR1)")                    \
    ZZ(PCISPC_BAR2,      PCICAP_FENCE,   0x18,    4,    "PCI hdr index/data pair register (BAR2)")          \
    ZZ(PCISPC_BAR3,      PCICAP_FENCE,   0x1c,    5,    "PCI hdr reserved area (BAR3)")                     \
    ZZ(PCISPC_BAR4,      PCICAP_FENCE,   0x20,    4,    "PCI hdr vendor specific register (BAR4)")          \
    ZZ(PCISPC_BAR5,      PCICAP_FENCE,   0x24,    4,    "PCI hdr vendor specific register (BAR5)")          \
    ZZ(PCISPC_CCPTR,     PCICAP_FENCE,   0x28,    4,    "PCI hdr CardBus CIS register (CCPTR)")             \
    ZZ(PCISPC_SS,        PCICAP_FENCE,   0x2c,    4,    "PCI hdr subsystem ID register (SS)")               \
    ZZ(PCISPC_EROM,      PCICAP_FENCE,   0x30,    4,    "PCI hdr expansion ROM register (EROM)")            \
    ZZ(PCISPC_CAP,       PCICAP_FENCE,   0x34,    1,    "PCI hdr CAP ptr register (CAP)")                   \
    ZZ(PCISPC_R,         PCICAP_FENCE,   0x35,    7,    "PCI hdr reserved area")                            \
    ZZ(PCISPC_INTR,      PCICAP_FENCE,   0x3c,    2,    "PCI hdr interrupt register (INTR)")                \
    ZZ(PCISPC_MGNT,      PCICAP_FENCE,   0x3e,    1,    "PCI hdr min grant register (MGMT)")                \
    ZZ(PCISPC_MLAT,      PCICAP_FENCE,   0x3f,    1,    "PCI hdr max latency register (MLAT)")              \
    ZZ(PCISPC_PID,       PCICAP_PMCAP,   INT_MAX, 2,    "PCI power mgmt ID register (PID)")                 \
    ZZ(PCISPC_PC,        PCICAP_PMCAP,   INT_MAX, 2,    "PCI power mgmt CAP register (PC)")                 \
    ZZ(PCISPC_PMCS,      PCICAP_PMCAP,   INT_MAX, 2,    "PCI power mgmt ctrl & status reg (PMCS)")          \
    ZZ(PCISPC_MID,       PCICAP_MSICAP,  INT_MAX, 2,    "PCI MSI ID register (MID)")                        \
    ZZ(PCISPC_MC,        PCICAP_MSICAP,  INT_MAX, 2,    "PCI MSI msg ctrl register (MC)")                   \
    ZZ(PCISPC_MA,        PCICAP_MSICAP,  INT_MAX, 4,    "PCI MSI msg addr  register (MA)")                  \
    ZZ(PCISPC_MUA,       PCICAP_MSICAP,  INT_MAX, 4,    "PCI MSI msg upper addr register (MUA)")            \
    ZZ(PCISPC_MD,        PCICAP_MSICAP,  INT_MAX, 2,    "PCI MSI msg data register (MD)")                   \
    ZZ(PCISPC_MMASK,     PCICAP_MSICAP,  INT_MAX, 4,    "PCI MSI mask bits register (MMASK)")               \
    ZZ(PCISPC_MPEND,     PCICAP_MSICAP,  INT_MAX, 4,    "PCI MSI pending bits register (MPEND)")            \
    ZZ(PCISPC_MXID,      PCICAP_MSIXCAP, INT_MAX, 2,    "PCI MSI-X ID register (MXID)")                     \
    ZZ(PCISPC_MXC,       PCICAP_MSIXCAP, INT_MAX, 2,    "PCI MSI-X msg ctrl register (MXC)")                \
    ZZ(PCISPC_MTAB,      PCICAP_MSIXCAP, INT_MAX, 4,    "PCI MSI-X table offset/BIR register (MTAB)")       \
    ZZ(PCISPC_MPBA,      PCICAP_MSIXCAP, INT_MAX, 4,    "PCI MSI-X PBA offset/BIR register (MPBA)")         \
    ZZ(PCISPC_PXID,      PCICAP_PXCAP,   INT_MAX, 2,    "PCI express CAP ID register (PXID)")               \
    ZZ(PCISPC_PXCAP,     PCICAP_PXCAP,   INT_MAX, 2,    "PCI express CAP register (PXCAP)")                 \
    ZZ(PCISPC_PXDCAP,    PCICAP_PXCAP,   INT_MAX, 4,    "PCI express device CAP register (PXDCAP)")         \
    ZZ(PCISPC_PXDC,      PCICAP_PXCAP,   INT_MAX, 2,    "PCI express device ctrl register (PXDC)")          \
    ZZ(PCISPC_PXDS,      PCICAP_PXCAP,   INT_MAX, 2,    "PCI express device status register (PXDS)")        \
    ZZ(PCISPC_PXLCAP,    PCICAP_PXCAP,   INT_MAX, 4,    "PCI express link CAP register (PXLCAP)")           \
    ZZ(PCISPC_PXLC,      PCICAP_PXCAP,   INT_MAX, 2,    "PCI express link ctrl register (PXLC)")            \
    ZZ(PCISPC_PXLS,      PCICAP_PXCAP,   INT_MAX, 2,    "PCI express link status register (PXLS)")          \
    ZZ(PCISPC_PXDCAP2,   PCICAP_PXCAP,   INT_MAX, 4,    "PCI express device CAP2 register (PXDCAP2)")       \
    ZZ(PCISPC_PXDC2,     PCICAP_PXCAP,   INT_MAX, 2,    "PCI express device ctrl2 register (PXDC2)")        \
    ZZ(PCISPC_AERID,     PCICAP_AERCAP,  INT_MAX, 4,    "PCI AER CAP ID register (AERID)")                  \
    ZZ(PCISPC_AERUCES,   PCICAP_AERCAP,  INT_MAX, 4,    "PCI AER uncorrect err stat register (AERECES)")    \
    ZZ(PCISPC_AERUCEM,   PCICAP_AERCAP,  INT_MAX, 4,    "PCI AER uncorrect err mask register (AERUCEM)")    \
    ZZ(PCISPC_AERUCESEV, PCICAP_AERCAP,  INT_MAX, 4,    "PCI AER uncorrect err severity reg (AERECESEV)")   \
    ZZ(PCISPC_AERCES,    PCICAP_AERCAP,  INT_MAX, 4,    "PCI AER correctable err stat reg (AERCES)")        \
    ZZ(PCISPC_AERCEM,    PCICAP_AERCAP,  INT_MAX, 4,    "PCI AER correctable err mask reg (AERCEM)")        \
    ZZ(PCISPC_AERCC,     PCICAP_AERCAP,  INT_MAX, 4,    "PCI AER adv err CAP & ctrl register AERCC)")       \
    ZZ(PCISPC_AERHL,     PCICAP_AERCAP,  INT_MAX, 16,   "PCI AER header log register (AERHL)")              \
    ZZ(PCISPC_AERTLP,    PCICAP_AERCAP,  INT_MAX, 16,   "PCI AER TLP prefix log register (AERTLP)")

#define ZZ(a, b, c, d, e)       a,
typedef enum PciSpc
{
    PCISPC_TABLE
    PCISPC_FENCE                // always must be the last element
} PciSpc;
#undef ZZ

struct PciSpcType {
    PciCapabilities cap;
    int             offset;     // ==INT_MAX implies offset is unknown
    int             size;
    const char      *desc;
};


//------------------------------------------------------------------------------
/*    CtlSpc,            offset,  size, desc */
#define CTLSPC_TABLE                                                                    \
    ZZ(CTLSPC_CAP,       0x00,    8,    "ctrlr CAP register (CAP)")                     \
    ZZ(CTLSPC_VS,        0x08,    4,    "ctrlr version register (VS)")                  \
    ZZ(CTLSPC_INTMS,     0x0c,    4,    "ctrlr interrupt mask set register (INTMS)")    \
    ZZ(CTLSPC_INTMC,     0x10,    4,    "ctrlr interrupt mask clear register (INTMC)")  \
    ZZ(CTLSPC_CC,        0x14,    4,    "ctrlr config register (CC)")                   \
    ZZ(CTLSPC_RES0,      0x18,    4,    "ctrlr reserved area 0x18")                     \
    ZZ(CTLSPC_CSTS,      0x1c,    4,    "ctrlr status register (CSTS)")                 \
    ZZ(CTLSPC_RES1,      0x20,    4,    "ctrlr reserved area 0x20")                     \
    ZZ(CTLSPC_AQA,       0x24,    4,    "ctrlr admin Q attrib register (AQA)")          \
    ZZ(CTLSPC_ASQ,       0x28,    8,    "ctrlr admin SQ BAR register (ASQ)")            \
    ZZ(CTLSPC_ACQ,       0x30,    8,    "ctrlr admin CQ BAR register (ACQ)")            \
    ZZ(CTLSPC_RES2,      0x38,    3784, "ctrlr reserved area 0x38")                     \
    ZZ(CTLSPC_RES3,      0xf00,   256,  "ctrlr cmd set specific (reserved)")
    // NOTE: doorbell registers are to numerous to handle in this manner.

#define ZZ(a, b, c, d)      a,
typedef enum CtlSpc
{
    CTLSPC_TABLE
    CTLSPC_FENCE        // always must be the last element
} CtlSpc;
#undef ZZ

struct CtlSpcType {
    int         offset;     // ==INT_MAX implies offset is unknown
    int         size;
    const char *desc;
};


//------------------------------------------------------------------------------
typedef enum STSBits {
    STS_RES0    = 0x0007,
    STS_IS      = 0x0008,
    STS_CL      = 0x0010,
    STS_C66     = 0x0020,
    STS_RES1    = 0x0040,
    STS_FBC     = 0x0080,
    STS_DPD     = 0x0100,
    STS_DEVT    = 0x0600,
    STS_STA     = 0x0800,
    STS_RTA     = 0x1000,
    STS_RMA     = 0x2000,
    STS_SSE     = 0x4000,
    STS_DPE     = 0x8000
} STSBits;


#endif
