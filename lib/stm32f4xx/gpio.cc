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

void Gpio::set_alternate_function(HalfWord mask, unsigned af) {
  af &= 0xF;

  auto val_low = read_afrl();
  auto val_high = read_afrh();

  for (unsigned i = 0; i < 8; ++i) {
    if (mask & 1) val_low = val_low.with_af(i, af);
    if (mask & (1 << 8)) val_high = val_high.with_af(i, af);
    mask >>= 1;
  }

  write_afrl(val_low);
  write_afrh(val_high);
}

void Gpio::set(HalfWord mask) {
  write_bsrr(bsrr_value_t().with_setbits(mask));
}

void Gpio::clear(HalfWord mask) {
  write_bsrr(bsrr_value_t().with_resetbits(mask));
}

void Gpio::toggle(HalfWord mask) {
  auto bits = read_odr().get_bits();
  write_bsrr(bsrr_value_t()
             .with_setbits(~bits & mask)
             .with_resetbits(bits & mask));
}

}  // namespace stm32f4xx
