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

#ifndef _GETLOGPAGEERRDEFS_H_
#define _GETLOGPAGEERRDEFS_H_


struct GetLogPageDataType {
    uint16_t    offset;        // byte offset into the returned data struct
    uint16_t    length;        // number of bytes this field consumes
    const char *desc;          // this fields formal description
};


/*     ErrLog,              offset, length, desc                           */
#define ERRLOG_TABLE                                                        \
    ZZ(ERRLOG_ERRCNT,       0,      8,      "Error count")                  \
    ZZ(ERRLOG_SQID,         8,      2,      "Submission queue ID")          \
    ZZ(ERRLOG_CMDID,        10,     2,      "Command ID")                   \
    ZZ(ERRLOG_STATCODE,     12,     2,      "Status code")                  \
    ZZ(ERRLOG_PARAMERRLOC,  14,     2,      "Parameter error location")     \
    ZZ(ERRLOG_LBA,          16,     8,      "LBA")                          \
    ZZ(ERRLOG_NAMSPC,       24,     4,      "Namespace")                    \
    ZZ(ERRLOG_VENDSPEC,     28,     1,      "Vendor specific information")  \
    ZZ(ERRLOG_RES0,         29,     35,     "Reserved area @ 0x1d")

#define ZZ(a, b, c, d)         a,
typedef enum ErrLog
{
    ERRLOG_TABLE
    ERRLOG_FENCE               // always must be the last element
} ErrLog;
#undef ZZ


/*     SmartLog,            offset, length, desc                           */
#define SMRTLOG_TABLE                                                        \
    ZZ(SMRTLOG_CRITWARN,    0,      1,      "Critical warning")             \
    ZZ(SMRTLOG_TEMP,        1,      2,      "Temperature")                  \
    ZZ(SMRTLOG_AVAILSPR,    3,      1,      "Available spare")              \
    ZZ(SMRTLOG_AVAILTHRES,  4,      1,      "Available spare threshold")    \
    ZZ(SMRTLOG_PCNTUSE,     5,      1,      "Percentage used")              \
    ZZ(SMRTLOG_RES0,        6,      26,     "Reserved area @ 0x06 ")        \
    ZZ(SMRTLOG_UNITRD,      32,     16,     "Data units read")              \
    ZZ(SMRTLOG_UNITWR,      48,     16,     "Data units written")           \
    ZZ(SMRTLOG_HOSTRD,      64,     16,     "Host read cmds")               \
    ZZ(SMRTLOG_HOSTWR,      80,     16,     "Host write cmds")              \
    ZZ(SMRTLOG_CTRLRBUSY,   96,     16,     "Ctrlr busy time")              \
    ZZ(SMRTLOG_POWCYCL,     112,    16,     "Power cycles")                 \
    ZZ(SMRTLOG_POWHRS,      128,    16,     "Power on hours")               \
    ZZ(SMRTLOG_UNSAFESHUT,  144,    16,     "Unsafe shutdowns")             \
    ZZ(SMRTLOG_MEDIAERR,    160,    16,     "Media errors")                 \
    ZZ(SMRTLOG_NUMLOGENTRY, 176,    16,     "Num err info log entries")     \
    ZZ(SMRTLOG_RES1,        192,    320,    "Reserved area @ 0xc0")

#define ZZ(a, b, c, d)         a,
typedef enum SmartLog
{
    SMRTLOG_TABLE
    SMRTLOG_FENCE               // always must be the last element
} SmartLog;
#undef ZZ


/*     FwLog,               offset, length, desc                           */
#define FWLOG_TABLE                                                         \
    ZZ(FWLOG_AFI,           0,      1,      "Active firmware info")         \
    ZZ(FWLOG_RES0,          1,      7,      "Reserved area @ 0x01")         \
    ZZ(FWLOG_FRS1,          8,      8,      "FW rev for slot 1")            \
    ZZ(FWLOG_FRS2,          16,     8,      "FW rev for slot 2")            \
    ZZ(FWLOG_FRS3,          24,     8,      "FW rev for slot 3")            \
    ZZ(FWLOG_FRS4,          32,     8,      "FW rev for slot 4")            \
    ZZ(FWLOG_FRS5,          40,     8,      "FW rev for slot 5")            \
    ZZ(FWLOG_FRS6,          48,     8,      "FW rev for slot 6")            \
    ZZ(FWLOG_FRS7,          56,     8,      "FW rev for slot 7")            \
    ZZ(FWLOG_RES1,          64,     448,    "Reserved area @ 0x40")

#define ZZ(a, b, c, d)         a,
typedef enum FwLog
{
    FWLOG_TABLE
    FWLOG_FENCE               // always must be the last element
} FwLog;
#undef ZZ

struct ParamErrLocFormat {
    uint8_t     ByteInCmd;
    uint8_t     BitInCmd : 3;
    uint8_t     RES      : 5;
} __attribute__((__packed__));

struct ErrLogStruct {
    uint64_t    ErrorCount;
    uint16_t    SQID;
    uint16_t    CID;
    uint16_t    STC;
    struct ParamErrLocFormat ParamErrLoc;
    uint64_t    LBA;
    uint32_t    Namspc;
    uint8_t     VSInfo;
    uint8_t     RES[35];
} __attribute__((__packed__));

struct SmartHealthLogStruct {
    uint8_t     CriticalWarn;
    uint16_t    Temperature;
    uint8_t     AvailSpare;
    uint8_t     AvailSpareThres;
    uint8_t     PercentageUsed;
    uint8_t     RES_06[26];
    uint64_t    DataUnitRead[2];
    uint64_t    DataUnitWritten[2];;
    uint64_t    HostReadCmds[2];
    uint64_t    HostWriteCmds[2];
    uint64_t    CtrlrBusyTime[2];
    uint64_t    PowerCycles[2];
    uint64_t    PowerOnHours[2];
    uint64_t    UnsafeShutdowns[2];
    uint64_t    MediaErrors[2];
    uint64_t    NumErrLogs[2];
    uint8_t     RES_C0[320];
}  __attribute__((__packed__));;

struct FWSlotInfoStruct {
    uint8_t     AFI;
    uint8_t     RES_01[7];
    uint64_t    FRS1;
    uint64_t    FRS2;
    uint64_t    FRS3;
    uint64_t    FRS4;
    uint64_t    FRS5;
    uint64_t    FRS6;
    uint64_t    FRS7;
    uint8_t     RES_40[448];
}  __attribute__((__packed__));;

#endif
