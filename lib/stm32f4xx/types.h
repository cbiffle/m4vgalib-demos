#ifndef LIB_STM32F4XX_TYPES_H
#define LIB_STM32F4XX_TYPES_H

#include "etl/armv7m/types.h"

/*
 * Re-export ARMv7-M types in the stm32f4xx namespace for convenience.
 */

namespace stm32f4xx {

using etl::armv7m::DoubleWord;
using etl::armv7m::Word;
using etl::armv7m::HalfWord;
using etl::armv7m::Byte;

}  // namespace stm32f4xx

#endif  // LIB_STM32F4XX_TYPES_H
