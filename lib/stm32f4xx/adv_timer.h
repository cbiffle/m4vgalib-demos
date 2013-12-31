#ifndef LIB_STM32F4XX_ADV_TIMER_H
#define LIB_STM32F4XX_ADV_TIMER_H

#include "lib/stm32f4xx/types.h"

namespace stm32f4xx {

struct AdvTimer {
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

  #define BFF_DEFINITION_FILE lib/stm32f4xx/adv_timer.reg
  #include <biffield/generate.h>
  #undef BFF_DEFINITION_FILE
};

extern AdvTimer tim1;
extern AdvTimer tim8;

}  // namespace stm32f4xx

#endif  // LIB_STM32F4XX_ADV_TIMER_H
