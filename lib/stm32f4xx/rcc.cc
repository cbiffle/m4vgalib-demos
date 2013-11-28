#include "lib/stm32f4xx/rcc.h"
#include "lib/armv7m/instructions.h"

namespace stm32f4xx {

void Rcc::enter_reset(ApbPeripheral p) {
  unsigned bus = get_bus_index(p);
  unsigned slot = get_slot_index(p);

  switch (bus) {
    case 0: write_apb1rstr(read_apb1rstr().with_bit(slot, true)); break;
    case 1: write_apb2rstr(read_apb2rstr().with_bit(slot, true)); break;
  }
  armv7m::data_synchronization_barrier();
}

void Rcc::leave_reset(ApbPeripheral p) {
  unsigned bus = get_bus_index(p);
  unsigned slot = get_slot_index(p);

  switch (bus) {
    case 0: write_apb1rstr(read_apb1rstr().with_bit(slot, false)); break;
    case 1: write_apb2rstr(read_apb2rstr().with_bit(slot, false)); break;
  }
  armv7m::data_synchronization_barrier();
}

void Rcc::enable_clock(ApbPeripheral p) {
  unsigned bus = get_bus_index(p);
  unsigned slot = get_slot_index(p);

  switch (bus) {
    case 0: write_apb1enr(read_apb1enr().with_bit(slot, true)); break;
    case 1: write_apb2enr(read_apb2enr().with_bit(slot, true)); break;
  }
  armv7m::data_synchronization_barrier();
}

void Rcc::disable_clock(ApbPeripheral p) {
  unsigned bus = get_bus_index(p);
  unsigned slot = get_slot_index(p);

  switch (bus) {
    case 0: write_apb1enr(read_apb1enr().with_bit(slot, false)); break;
    case 1: write_apb2enr(read_apb2enr().with_bit(slot, false)); break;
  }
  armv7m::data_synchronization_barrier();
}

}  // namespace stm32f4xx
