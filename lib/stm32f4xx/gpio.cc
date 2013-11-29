#include "lib/stm32f4xx/gpio.h"

namespace stm32f4xx {

/*
 * Each of the GPIO registers gets a very similar mutator.  Generate these
 * with a macro to avoid copy-pasting.
 */

#define GPIO_MUTATOR(name, type, reg, field) \
  void Gpio::set_ ## name (HalfWord mask, type x) { \
    auto val = read_ ## reg(); \
    for (unsigned i = 0; i < 16; ++i) { \
      if (mask & 1) val = val.with_ ## field(i, x); \
      mask >>= 1; \
    } \
    write_ ## reg(val); \
  }

GPIO_MUTATOR(mode, Mode, moder, mode)
GPIO_MUTATOR(output_type, OutputType, otyper, otype)
GPIO_MUTATOR(output_speed, OutputSpeed, ospeedr, ospeed)
GPIO_MUTATOR(pull, Pull, pupdr, pupd)

}  // namespace stm32f4xx
