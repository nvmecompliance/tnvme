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

#ifndef _TNVME_H_
#define _TNVME_H_

#include <stdio.h>
#include <string>
#include <vector>
#include "dnvme.h"
#include "testRef.h"

using namespace std;

// Identifies which file and line number in a compact form to save space
#define HERE        __FILE__, __LINE__

#define APPNAME         "tnvme"
#define LEVEL           APPNAME
#define LOG_NRM(fmt, ...)       \
    fprintf(stderr, "%s:%s:%d: " fmt "\n", LEVEL, HERE, ## __VA_ARGS__);
#define LOG_ERR(fmt, ...)       \
    fprintf(stderr, "%s-err:%s:%d: " fmt "\n", LEVEL, HERE, ## __VA_ARGS__);
#define LOG_WARN(fmt, ...)      \
    fprintf(stderr, "%s-warn:%s:%d: " fmt "\n", LEVEL, HERE, ## __VA_ARGS__);

#ifdef DEBUG
#define LOG_DBG(fmt, ...)       \
    fprintf(stderr, "%s-dbg:%s:%d: " fmt "\n", LEVEL, HERE, ## __VA_ARGS__);
#else
#define LOG_DBG(fmt, ...)       ;
#endif


#define MAX_CHAR_PER_LINE_DESCRIPTION       63

#define DFLT_TIMEOUT_ms                     10000   // 10s
#define PER_CMD_TIMEOUT_ms                  20
#define CALC_TIMEOUT_ms(numCmds)    \
    (((numCmds * PER_CMD_TIMEOUT_ms) > DFLT_TIMEOUT_ms) ?    \
    (numCmds * PER_CMD_TIMEOUT_ms) : DFLT_TIMEOUT_ms)

/**
 * When the namespace reports unlimited transfer size, this define artificially
 * caps it to allow reasonable coverage vs runtime target.
 */
#define MAX_DATA_TX_SIZE                    (256 * 1024)

#define UINT32_MAX                          ((uint32_t)-1)

#define MAX(a,b)                            (((a) > (b)) ? (a) : (b))
#define MIN(a,b)                            (((a) < (b)) ? (a) : (b))


typedef enum {
    SPECREV_10b,             // http://www.nvmexpress.org/ spec. rev. 1.0b
    SPECREVTYPE_FENCE        // always must be last element
} SpecRev;


typedef enum {
    DATAPAT_CONST_8BIT,
    DATAPAT_CONST_16BIT,
    DATAPAT_CONST_32BIT,
    DATAPAT_INC_8BIT,
    DATAPAT_INC_16BIT,
    DATAPAT_INC_32BIT,

    DATAPATTERN_FENCE           // always must be last element
} DataPattern;


/**
 * Combination/permutation not listed below should be considered illegal. The
 * last permutation listed, request spec'd test within spec'd group, causes
 * the zero, configuration, and sequence dependencies to become invoked, refer
 * to: https://github.com/nvmecompliance/tnvme/wiki/Test-Numbering
 *
 * req      group       xLevel      yLevel      zLevel    implies
 * -----------------------------------------------------------------------------
 * false     n/a         n/a         n/a         n/a      nothing has been requested
 * true   ==UINT_MAX     n/a         n/a         n/a      request all test within all all groups
 * true   !=UINT_MAX  ==UINT_MAX  ==UINT_MAX  ==UINT_MAX  request all test within spec'd group
 * true   !=UINT_MAX  !=UINT_MAX  !=UINT_MAX  !=UINT_MAX  request spec'd test within spec'd group
 */
struct TestTarget {
    bool            req;     // requested by cmd line
    TestRef         t;
};

struct RmmapIo {
    bool            req;     // requested by cmd line
    nvme_io_space   space;
    size_t          offset;
    size_t          size;
    nvme_acc_type   acc;
};

struct WmmapIo {
    bool            req;     // requested by cmd line
    nvme_io_space   space;
    size_t          offset;
    size_t          size;
    uint64_t        value;
    nvme_acc_type   acc;
};

struct NumQueues {
    bool            req;     // requested by cmd line
    uint16_t        ncqr;    // Number of IOCQ's requested
    uint16_t        nsqr;    // Number of IOSQ's requested
};

struct ErrorRegs {
    uint16_t        sts;     // PCI addr space STS regr bitmask
    uint16_t        pxds;    // PCI addr space PCICAP.PXDS reg bitmask
    uint16_t        aeruces; // PCI addr space AERCAP.AERUCES reg bitmask
    uint32_t        csts;    // Ctrl'r addr space CSTS reg bitmask
};

struct FormatDUT {
    uint32_t        nsid;    // The Format.DW1.NSID field to send cmd
    uint8_t         ses;     // Format NVM cmd; DW10.ses value
    uint8_t         pi;      // Format NVM cmd; DW10.pi value
    uint8_t         lbaf;    // Format NVM cmd; DW10.lbaf value
    bool            pil;     // Format NVM cmd; DW10.pil value
    bool            ms;      // Format NVM cmd; DW10.ms value
};

struct Format {
    bool                req;    // requested by cmd line
    vector<FormatDUT>   cmds;   // Array of format NVM cmds to issue
};

struct IdentifyDUT {
    uint32_t            nsid;   // The Identify.DW1.NSID field
    bool                cns;    // The Identify.DW10.CNS field
    vector<uint8_t>     raw;    // Raw identify data payload to compare against
    vector<uint8_t>     mask;   // Mask set bits indicate raw[x] bits to compare
};

struct Golden {
    bool                req;    // Requested by cmd line
    string              outputFile;
    vector<IdentifyDUT> cmds;   // Array of identify cmd data to validate
};

struct FWImage {
    bool                req;    // Requested by cmd line
    vector<uint8_t>     data;   // Array of raw FW binary bytes to program
};


struct CmdLine {
    bool            summary;
    bool            ignore;
    bool            reset;
    bool            restore;
    bool            postfail;
    bool            rsvdfields;
    bool            preserve;
    size_t          loop;
    SpecRev         rev;
    TestTarget      detail;
    TestTarget      test;
    string          device;
    vector<TestRef> skiptest;
    Format          format;
    Golden          golden;
    FWImage         fwImage;
    RmmapIo         rmmap;
    WmmapIo         wmmap;
    NumQueues       numQueues;
    ErrorRegs       errRegs;
    string          dump;
};


#endif
