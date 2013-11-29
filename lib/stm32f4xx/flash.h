#ifndef LIB_STM32F4XX_FLASH_H
#define LIB_STM32F4XX_FLASH_H

namespace stm32f4xx {

struct Flash {
  #define BFF_DEFINITION_FILE lib/stm32f4xx/flash.reg
  #include <biffield/generate.h>
  #undef BFF_DEFINITION_FILE
};

extern Flash flash;

}  // namespace stm32f4xx

#endif  // LIB_STM32F4XX_FLASH_H
