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

using namespace std;

#define APPNAME         "tnvme"
#define LEVEL           APPNAME
#define LOG_NRM(fmt, ...)    \
    fprintf(stderr, "%s: " fmt "\n", LEVEL, ## __VA_ARGS__);
#define LOG_ERR(fmt, ...)    \
    fprintf(stderr, "%s-err:%s:%d: " fmt "\n", LEVEL, __FILE__, __LINE__,   \
        ## __VA_ARGS__);
#define LOG_WARN(fmt, ...)    \
    fprintf(stderr, "%s-warn:%s:%d: " fmt "\n", LEVEL, __FILE__, __LINE__,  \
        ## __VA_ARGS__);
#ifdef DEBUG
#define LOG_DBG(fmt, ...)    \
    fprintf(stderr, "%s-dbg:%s:%d: " fmt "\n", LEVEL, __FILE__, __LINE__,   \
        ## __VA_ARGS__);
#else
#define LOG_DBG(fmt, ...)    ;
#endif


#define MAX_CHAR_PER_LINE_DESCRIPTION       66


typedef enum {
    SPECREV_10b,                // http://www.nvmexpress.org/ spec. rev. 1.0b
    SPECREVTYPE_FENCE           // always must be last element
} SpecRev;

struct TestRef {
    size_t  group;
    size_t  major;
    size_t  minor;
    TestRef() {group = 0; major = 0; minor = 0; }
    TestRef(size_t g, size_t j, size_t n) { group = g; major = j, minor = n; }
};

/**
 * req      group          major         minor     implies
 * -----------------------------------------------------------------------------
 * false     n/a            n/a           n/a      nothing has been requested
 * true   ==UINT_MAX        n/a           n/a      request all groups all tests
 * true   !=UINT_MAX     ==UINT_MAX || ==UINT_MAX  request spec'd group
 * true   !=UINT_MAX     !=UINT_MAX && !=UINT_MAX  request spec'd test in group
 */
struct TestTarget {
    bool            req;    // requested by cmd line
    TestRef t;
};

struct RmmapIo {
    bool            req;    // requested by cmd line
    nvme_io_space   space;
    size_t          offset;
    size_t          size;
    nvme_acc_type   acc;
};

struct WmmapIo {
    bool            req;    // requested by cmd line
    nvme_io_space   space;
    size_t          offset;
    size_t          size;
    uint64_t        value;
    nvme_acc_type   acc;
};

struct Queues {
    bool            req;    // requested by cmd line
    uint16_t        ncqr;   // Number of IOCQ's requested
    uint16_t        nsqr;   // Number of IOSQ's requested
};

struct CmdLine {
    bool            summary;
    bool            ignore;
    bool            reset;
    size_t          loop;
    SpecRev         rev;
    TestTarget      detail;
    TestTarget      test;
    string          device;
    vector<TestRef> skiptest;
    RmmapIo         rmmap;
    WmmapIo         wmmap;
    Queues          queues;
};


#endif
