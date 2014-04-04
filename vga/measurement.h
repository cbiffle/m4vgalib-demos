#ifndef VGA_MEASUREMENT_H
#define VGA_MEASUREMENT_H

#include "lib/common/attribute_macros.h"
#include "etl/armv7m/sys_tick.h"
#include "lib/stm32f4xx/gpio.h"

namespace vga {

void msigs_init();

INLINE void msig_a_set() {
  stm32f4xx::gpioc.set(stm32f4xx::Gpio::p9);
}

INLINE void msig_a_clear() {
  stm32f4xx::gpioc.clear(stm32f4xx::Gpio::p9);
}

INLINE void msig_a_toggle() {
  stm32f4xx::gpioc.toggle(stm32f4xx::Gpio::p9);
}

void mtim_init();

INLINE unsigned mtim_get() {
  return etl::armv7m::sys_tick.read_cvr().get_current();
}

}  // namespace vga

#endif  // VGA_MEASUREMENT_H
