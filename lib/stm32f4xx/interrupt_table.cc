#include "lib/stm32f4xx/interrupts.h"
#include "lib/common/attribute_macros.h"

namespace stm32f4xx {

/*******************************************************************************
 * The exception table!
 */

typedef void (*InterruptHandler)(void);

struct InterruptTable {
  #define STM32F4XX_IRQ(name) InterruptHandler name ## _handler;
  #include "lib/stm32f4xx/interrupts.def"
  #undef STM32F4XX_IRQ
};

/*
 * There are a couple of nuances to the interrupt table definition.
 *
 *  1. We place it in a section called .stm32f4xx_interrupt_table.  There's
 *     nothing special about that name, except that the linker scripts look for
 *     it and put it in the right place.
 *
 *  2. We make the table const, ensuring that the linker will let us place it
 *     in Flash if the linker script wants it there.
 */
SECTION(".stm32f4xx_interrupt_table")
USED
InterruptTable const stm32f4xx_interrupt_table = {
  #define STM32F4XX_IRQ(name) stm32f4xx_ ## name ## _handler,
  #include "lib/stm32f4xx/interrupts.def"
  #undef STM32F4XX_IRQ
};

extern "C" {
  void unexpected_irq();  // hack
  void unexpected_irq() { while (1); }
  #define STM32F4XX_IRQ(name) void stm32f4xx_ ## name ## _handler() \
                              __attribute__((weak, alias("unexpected_irq")));
  #include "lib/stm32f4xx/interrupts.def"
  #undef STM32F4XX_IRQ
}

}  // namespace stm32f4xx
