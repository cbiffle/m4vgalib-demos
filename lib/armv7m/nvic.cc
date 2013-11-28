#include "lib/armv7m/nvic.h"
#include "lib/armv7m/instructions.h"

namespace armv7m {

void enable_irq(unsigned irq) {
  unsigned bank = irq / 32;
  unsigned index = irq % 32;

  nvic.iser[bank] = 1 << index;

  /*
   * The DSB is required to apply the change to the peripheral; the ISB, to
   * ensure the atomic enable behavior described in the header file.
   */
  data_synchronization_barrier();  // IRQ gets enabled now
  instruction_synchronization_barrier();  // IRQ executes now if pending
}

void disable_irq(unsigned irq) {
  unsigned bank = irq / 32;
  unsigned index = irq % 32;

  nvic.icer[bank] = 1 << index;
  data_synchronization_barrier();  // IRQ gets disabled now
  // No ISB required: if the IRQ has become active, it remains active.
}

void clear_pending_irq(unsigned irq) {
  unsigned bank = irq / 32;
  unsigned index = irq % 32;

  nvic.icpr[bank] = 1 << index;
  data_synchronization_barrier();  // IRQ gets disabled now
  instruction_synchronization_barrier();  // TODO(cbiffle): unnecessary?
}

void set_irq_priority(unsigned irq, Byte priority) {
  nvic.ipr_byte[irq] = priority;
  data_synchronization_barrier();  // IRQ gets prioritized now
  instruction_synchronization_barrier();  // IRQ executes now if suitable
}

}  // namespace armv7m
