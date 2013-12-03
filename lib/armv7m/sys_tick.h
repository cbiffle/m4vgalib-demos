#ifndef LIB_ARMV7M_SYS_TICK_H
#define LIB_ARMV7M_SYS_TICK_H

#include "lib/armv7m/types.h"

namespace armv7m {

struct SysTick {
  #define BFF_DEFINITION_FILE lib/armv7m/sys_tick.reg
  #include <biffield/generate.h>
  #undef BFF_DEFINITION_FILE
};

extern SysTick sys_tick;

}  // namespace armv7m

#endif  // LIB_ARMV7M_SYS_TICK_H
