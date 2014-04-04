#ifndef LIB_STM32F4XX_GP_TIMER_H
#define LIB_STM32F4XX_GP_TIMER_H

#include "etl/stm32f4xx/types.h"

namespace stm32f4xx {

struct GpTimer {
  typedef etl::stm32f4xx::Word Word;

  enum class OcMode {
    frozen = 0b000,
    ch1_active_on_match = 0b001,
    ch1_inactive_on_match = 0b010,
    toggle = 0b011,
    force_inactive = 0b100,
    force_active = 0b101,
    pwm1 = 0b110,
    pwm2 = 0b111,
  };

  #define ETL_BFF_DEFINITION_FILE lib/stm32f4xx/adv_timer.reg
  #include "etl/biffield/generate.h"
  #undef ETL_BFF_DEFINITION_FILE
};

extern GpTimer tim2;
extern GpTimer tim3;
extern GpTimer tim4;
extern GpTimer tim5;

}  // namespace stm32f4xx

#endif  // LIB_STM32F4XX_GP_TIMER_H
