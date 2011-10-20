#ifndef _CE_H_
#define _CE_H_

#include <stdint.h>


struct CEbyDW {
    uint32_t dw0;
    uint32_t dw1;
    uint32_t dw2;
    uint32_t dw3;
};

struct CEbyName {
    uint32_t cmdSpec;
    uint32_t reserved;
    uint16_t sqHdPtr;
    uint16_t sqId;
    uint16_t cmdId;
    uint16_t pBit   : 1;
    uint16_t status : 15;
};

/**
 * Completion Element (CE) definition.
 */
union CE {
    struct CEbyDW   d;
    struct CEbyName n;
};


#endif
