#ifndef LIB_STM32F4XX_SYSCFG_H
#define LIB_STM32F4XX_SYSCFG_H

namespace stm32f4xx {

struct SysCfg {
  #define BFF_DEFINITION_FILE lib/stm32f4xx/syscfg.reg
  #include <biffield/generate.h>
  #undef BFF_DEFINITION_FILE
};

extern SysCfg syscfg;

}  // namespace stm32f4xx

#endif  // LIB_STM32F4XX_SYSCFG_H
