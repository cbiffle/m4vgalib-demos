#include "lib/stm32f4xx/interrupts.h"

#include "lib/armv7m/nvic.h"
#include "lib/stm32f4xx/types.h"

namespace stm32f4xx {

void enable_irq(Interrupt irq) {
  armv7m::enable_irq(static_cast<unsigned>(irq));
}

void disable_irq(Interrupt irq) {
  armv7m::disable_irq(static_cast<unsigned>(irq));
}

void clear_pending_irq(Interrupt irq) {
  armv7m::clear_pending_irq(static_cast<unsigned>(irq));
}

void set_irq_priority(Interrupt irq, unsigned priority) {
  unsigned shifted_priority = (priority << 4) & 0xFF;
  armv7m::set_irq_priority(static_cast<unsigned>(irq),
                           static_cast<Byte>(shifted_priority));
}

}  // namespace stm32f4xx
