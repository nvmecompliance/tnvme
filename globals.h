#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include "Singletons/registers.h"

// NOTE: To make it easier to decifer objects which are global, prepend 'g'


/// Tests are encouraged to use this instance for all register access
extern Registers *gRegisters;


#endif
