#ifndef _TNVME_H_
#define _TNVME_H_

#include <stdio.h>

#define DEBUG

#define APPNAME         "tnvme"
#define LEVEL           APPNAME
#define LOG_NORM(fmt, ...)    \
    fprintf(stderr, "%s: " fmt "\n", LEVEL, ## __VA_ARGS__);
#define LOG_ERR(fmt, ...)    \
    fprintf(stderr, "%s-err:%s:%d: " fmt "\n", LEVEL, __FILE__, __LINE__, ## __VA_ARGS__);
#ifdef DEBUG
#define LOG_DBG(fmt, ...)    \
    fprintf(stderr, "%s-dbg:%s:%d: " fmt "\n", LEVEL, __FILE__, __LINE__, ## __VA_ARGS__);
#else
#define LOG_DBG(fmt, ...)    ;
#endif

#define MAX_CHAR_PER_LINE_DESCRIPTION       72

typedef enum
{
    SPECREV_10,                 // http://www.nvmexpress.org/ spec. rev. 1.0
    SPECREV_10a,                // http://www.nvmexpress.org/ spec. rev. 1.0a
    SPECREVTYPE_FENCE           // always must be last element
} SpecRevType;

typedef enum
{
    RESET_PCI,                  // via PXDCAP.FLRC bit
    RESET_CTRLR,                // via CC.EN bit
    RESETTYPE_FENCE             // always must be the last element
} ResetType;

/**
 * req      group          major         minor     implies
 * -----------------------------------------------------------------------------
 * false     n/a            n/a           n/a      nothing has been requested
 * true   ==ULONG_MAX       n/a           n/a      request all groups all tests
 * true   !=ULONG_MAX  ==ULONG_MAX || ==ULONG_MAX  request spec'dc group
 * true   !=ULONG_MAX  !=ULONG_MAX && !=ULONG_MAX  request spec'd test in group
 */
struct TargetType {
    bool    req;    // requested
    size_t  group;
    size_t  major;
    size_t  minor;
};

struct CmdLineType {
    bool        summary;
    bool        ignore;
    size_t      loop;
    SpecRevType rev;
    TargetType  detail;
    TargetType  test;
    ResetType   reset;
};


#endif
