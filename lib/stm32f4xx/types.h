#ifndef LIB_STM32F4XX_TYPES_H
#define LIB_STM32F4XX_TYPES_H

#include "lib/armv7m/types.h"

/*
 * Re-export ARMv7-M types in the stm32f4xx namespace for convenience.
 */

namespace stm32f4xx {

using armv7m::DoubleWord;
using armv7m::Word;
using armv7m::HalfWord;
using armv7m::Byte;

}  // namespace stm32f4xx

#endif  // LIB_STM32F4XX_TYPES_H
