#ifndef LIB_STM32F4XX_INTERRUPTS_H
#define LIB_STM32F4XX_INTERRUPTS_H

namespace stm32f4xx {

/*
 * A convenient enumeration of all STM32F4xx interrupts.
 */
enum class Interrupt : unsigned {
  #define STM32F4XX_IRQ(name) name,
  #include "lib/stm32f4xx/interrupts.def"
  #undef STM32F4XX_IRQ
};

/*
 * Facade for armv7m::enable_irq using the Interrupt enum.
 */
void enable_irq(Interrupt);

/*
 * Facade for armv7m::disable_irq using the Interrupt enum.
 */
void disable_irq(Interrupt);

/*
 * Facade for armv7m::clear_pending_irq using the Interrupt enum.
 */
void clear_pending_irq(Interrupt);

/*
 * Facade for armv7m::disable_irq using the Interrupt enum *and* STM32F4xx
 * priorities.
 *
 * The STM32F4xx only implements four bits of priority, so valid priorities
 * range from 0 - 15.
 *
 * Precondition: priority is valid.
 */
void set_irq_priority(Interrupt, unsigned priority);

}  // namespace stm32f4xx

extern "C" {
  #define STM32F4XX_IRQ(name) void stm32f4xx_ ## name ## _handler();
  #include "lib/stm32f4xx/interrupts.def"
  #undef STM32F4XX_IRQ
}

#endif  // LIB_STM32F4XX_INTERRUPTS_H
