#ifndef LIB_STM32F4XX_SPI_H
#define LIB_STM32F4XX_SPI_H

#include "lib/stm32f4xx/types.h"

namespace stm32f4xx {

struct Spi {
  #define BFF_DEFINITION_FILE lib/stm32f4xx/spi.reg
  #include <biffield/generate.h>
  #undef BFF_DEFINITION_FILE
};

extern Spi spi3;

}  // namespace stm32f4xx

#endif  // LIB_STM32F4XX_SPI_H
