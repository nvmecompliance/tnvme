#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include "dnvme.h"
#include "Singletons/registers.h"
#include "Singletons/rsrcMngr.h"
#include "Singletons/ctrlrConfig.h"

// NOTE: To make it easier to decifer objects which are global, prepend 'g'

extern struct metrics_driver gDriverMetrics;

/// Tests are encouraged to use this instance for all register access
extern Registers *gRegisters;
/// Tests are encouraged to use this instance to allocate test resources
extern RsrcMngr *gRsrcMngr;
/// Tests are encouraged to use this instance to interface with ctrlr config
extern CtrlrConfig *gCtrlrConfig;


#endif
