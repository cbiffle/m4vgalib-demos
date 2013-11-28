#ifndef LIB_STM32F4XX_RCC_H
#define LIB_STM32F4XX_RCC_H

#include "lib/stm32f4xx/types.h"
#include "lib/stm32f4xx/ahb.h"
#include "lib/stm32f4xx/apb.h"

namespace stm32f4xx {

struct Rcc {
  enum class pprex_t : unsigned {
    div1 = 0b000,
    div2 = 0b100,
    div4 = 0b101,
    div8 = 0b110,
    div16 = 0b111,
  };

  #define BFF_DEFINITION_FILE lib/stm32f4xx/rcc.reg
  #include <biffield/generate.h>
  #undef BFF_DEFINITION_FILE

  void enter_reset(AhbPeripheral);
  void enter_reset(ApbPeripheral);

  void leave_reset(AhbPeripheral);
  void leave_reset(ApbPeripheral);

  void enable_clock(AhbPeripheral);
  void enable_clock(ApbPeripheral);

  void disable_clock(AhbPeripheral);
  void disable_clock(ApbPeripheral);
};

extern Rcc rcc;

}  // namespace stm32f4xx

#endif  // LIB_STM32F4XX_RCC_H
