#ifndef LIB_STM32F4XX_DBG_H
#define LIB_STM32F4XX_DBG_H

#include "lib/stm32f4xx/types.h"

namespace stm32f4xx {

struct Dbg {
  #define BFF_DEFINITION_FILE lib/stm32f4xx/dbg.reg
  #include <biffield/generate.h>
  #undef BFF_DEFINITION_FILE
};

extern Dbg dbg;

}  // namespace stm32f4xx

#endif  // LIB_STM32F4XX_DBG_H
