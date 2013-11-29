#ifndef LIB_STM32F4XX_GPIO_H
#define LIB_STM32F4XX_GPIO_H

#include "lib/stm32f4xx/types.h"

namespace stm32f4xx {

struct Gpio {
  enum class Mode {
    input     = 0b00,
    gpio      = 0b01,
    alternate = 0b10,
    analog    = 0b11,
  };

  enum class OutputType {
    push_pull = 0,
    open_drain = 1,
  };

  enum class OutputSpeed {
    low_2mhz     = 0b00,
    medium_25mhz = 0b01,
    fast_50mhz   = 0b10,
    high_100mhz  = 0b11,
  };

  enum class Pull {
    none = 0b00,
    up   = 0b01,
    down = 0b10,
    // 0b11 is reserved
  };

  #define BFF_DEFINITION_FILE lib/stm32f4xx/gpio.reg
  #include <biffield/generate.h>
  #undef BFF_DEFINITION_FILE

  /*
   * Changes the mode of each pin whose corresponding bit in the mask is 1.
   */
  void set_mode(HalfWord mask, Mode);

  /*
   * Changes the output type of each pin whose corresponding bit in the mask is
   * 1.
   */
  void set_output_type(HalfWord mask, OutputType);

  /*
   * Changes the output speed of each pin whose corresponding bit in the mask is
   * 1.
   */
  void set_output_speed(HalfWord mask, OutputSpeed);

  /*
   * Changes the pull of each pin whose corresponding bit in the mask is
   * 1.
   */
  void set_pull(HalfWord mask, Pull);
};

extern Gpio gpioa;
extern Gpio gpiob;
extern Gpio gpioc;
extern Gpio gpiod;
extern Gpio gpioe;

}  // namespace stm32f4xx

#endif  // LIB_STM32F4XX_GPIO_H
