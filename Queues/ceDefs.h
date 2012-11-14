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

#ifndef _CEDEFS_H_
#define _CEDEFS_H_

typedef enum SCT {
    SCT_GENERIC = 0x00,
    SCT_CMD     = 0x01,
    SCT_MEDIA   = 0x02,
    SCT_VENDOR  = 0x07
} SCT;


/*     CEStat,                   sct,         sc,   desc */
#define CESTAT_TABLE                                                                        \
    ZZ(CESTAT_SUCCESS,           SCT_GENERIC, 0x00, "The cmd completed successfully")       \
    ZZ(CESTAT_INVAL_OPCODE,      SCT_GENERIC, 0x01, "Invalid cmd opcode")                   \
    ZZ(CESTAT_INVAL_FIELD,       SCT_GENERIC, 0x02, "Invalid field in cmd")                 \
    ZZ(CESTAT_ID_CONFLICT,       SCT_GENERIC, 0x03, "Cmd ID conflict")                      \
    ZZ(CESTAT_XFER_ERR,          SCT_GENERIC, 0x04, "Data transfer error")                  \
    ZZ(CESTAT_ABRT_PWR_LOSS,     SCT_GENERIC, 0x05, "Cmd aborted due to power loss notify") \
    ZZ(CESTAT_INTERN_ERR,        SCT_GENERIC, 0x06, "Internal device error")                \
    ZZ(CESTAT_ABRT_REQ,          SCT_GENERIC, 0x07, "Cmd abort requested")                  \
    ZZ(CESTAT_ABRT_SQ_DEL,       SCT_GENERIC, 0x08, "Cmd aborted due to SQ deletion")       \
    ZZ(CESTAT_ABRT_FAIL_FUSE,    SCT_GENERIC, 0x09, "Cmd aborted due to failed fused cmd")  \
    ZZ(CESTAT_ABRT_MISS_FUSE,    SCT_GENERIC, 0x0a, "Cmd aborted due to missing fused cmd") \
    ZZ(CESTAT_INVAL_NAMSPC,      SCT_GENERIC, 0x0b, "Invalid namespace or format")         \
    ZZ(CESTAT_LBA_OUT_RANGE,     SCT_GENERIC, 0x80, "LBA out of range")                     \
    ZZ(CESTAT_CAP_EXCEEDED,      SCT_GENERIC, 0x81, "Capacity exceeded")                    \
    ZZ(CESTAT_NAMSPC_NOT_RDY,    SCT_GENERIC, 0x82, "Namespace not ready")                  \
    ZZ(CESTAT_CQ_INVALID,        SCT_CMD,     0x00, "Completion Q invalid")                 \
    ZZ(CESTAT_INVALID_QID,       SCT_CMD,     0x01, "Invalid queue ID")                     \
    ZZ(CESTAT_MAX_Q_SIZE_EXCEED, SCT_CMD,     0x02, "Maximum queue size exceeded")          \
    ZZ(CESTAT_ABORT_CMD_LIMIT,   SCT_CMD,     0x03, "Abort cmd limit exceeded")             \
    ZZ(CESTAT_ASYNC_REQ_EXCEED,  SCT_CMD,     0x05, "Async request limit exceeded")         \
    ZZ(CESTAT_INVAL_FIRM_SLOT,   SCT_CMD,     0x06, "Invalid firmware slot")                \
    ZZ(CESTAT_INVAL_FIRM_IMAGE,  SCT_CMD,     0x07, "Invalid firmware image")               \
    ZZ(CESTAT_INVAL_INT_VEC,     SCT_CMD,     0x08, "Invalid interrupt vector")             \
    ZZ(CESTAT_INVAL_LOG_PAGE,    SCT_CMD,     0x09, "Invalid log page")                     \
    ZZ(CESTAT_INVAL_FORMAT,      SCT_CMD,     0x0a, "Invalid format")                       \
    ZZ(CESTAT_CONFLICT_ATTR,     SCT_CMD,     0x80, "Conflicting attributes")               \
    ZZ(CESTAT_INVAL_PROT_INFO,   SCT_CMD,     0x81, "Invalid protection information")       \
    ZZ(CESTAT_WRITE_INTO_RO,     SCT_CMD,     0x82, "Attempt to write to read only range")  \
    ZZ(CESTAT_WRITE_FAULT,       SCT_MEDIA,   0x80, "Write fault")                          \
    ZZ(CESTAT_UNRECOVER_RD_ERR,  SCT_MEDIA,   0x81, "Unrecovered read error")               \
    ZZ(CESTAT_E2E_GUARD_CHK_ERR, SCT_MEDIA,   0x82, "End-to-end guard check error")         \
    ZZ(CESTAT_E2E_APP_TAG_ERR,   SCT_MEDIA,   0x83, "End-to-end app tag check error")       \
    ZZ(CESTAT_E2E_REF_TAG_ERR,   SCT_MEDIA,   0x84, "End-to-end reference tag check error") \
    ZZ(CESTAT_COMPARE_FAIL,      SCT_MEDIA,   0x85, "Compare failure")                      \
    ZZ(CESTAT_ACCESS_DENIED,     SCT_MEDIA,   0x86, "Access denied")

#define ZZ(a, b, c, d)      a,
typedef enum CEStat
{
    CESTAT_TABLE
    CESTAT_FENCE              // always must be the last element
} CEStat;
#undef ZZ

struct CEStatType {
    SCT         sct;    // Status Code Type (sct)
    uint8_t     sc;     // Status Code (sc)
    const char *desc;
};


#endif
