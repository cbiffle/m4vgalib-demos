#ifndef LIB_STM32F4XX_IIC_H
#define LIB_STM32F4XX_IIC_H

namespace stm32f4xx {

struct Iic {
  #define BFF_DEFINITION_FILE lib/stm32f4xx/iic.reg
  #include <biffield/generate.h>
  #undef BFF_DEFINITION_FILE
};

extern Iic iic1;

}  // namespace stm32f4xx

#endif  // LIB_STM32F4XX_IIC_H
