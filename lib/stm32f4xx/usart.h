#ifndef LIB_STM32F4XX_USART_H
#define LIB_STM32F4XX_USART_H

#include "etl/stm32f4xx/types.h"

namespace stm32f4xx {

struct Usart {
  typedef etl::stm32f4xx::Word Word;

  #define ETL_BFF_DEFINITION_FILE lib/stm32f4xx/usart.reg
  #include "etl/biffield/generate.h"
  #undef ETL_BFF_DEFINITION_FILE
};

extern Usart usart1;
extern Usart usart2;
extern Usart usart3;
extern Usart usart6;

}  // namespace stm32f4xx

#endif  // LIB_STM32F4XX_USART_H
