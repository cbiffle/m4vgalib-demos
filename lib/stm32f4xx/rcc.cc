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


void Rcc::enter_reset(AhbPeripheral p) {
  unsigned bus = get_bus_index(p);
  unsigned slot = get_reset_index(p);

  switch (bus) {
    case 0: write_ahb1rstr(read_ahb1rstr().with_bit(slot, true)); break;
    case 1: write_ahb2rstr(read_ahb2rstr().with_bit(slot, true)); break;
    case 2: write_ahb3rstr(read_ahb3rstr().with_bit(slot, true)); break;
  }
  armv7m::data_synchronization_barrier();
}

void Rcc::leave_reset(AhbPeripheral p) {
  unsigned bus = get_bus_index(p);
  unsigned slot = get_reset_index(p);

  switch (bus) {
    case 0: write_ahb1rstr(read_ahb1rstr().with_bit(slot, false)); break;
    case 1: write_ahb2rstr(read_ahb2rstr().with_bit(slot, false)); break;
    case 2: write_ahb3rstr(read_ahb3rstr().with_bit(slot, false)); break;
  }
  armv7m::data_synchronization_barrier();
}

void Rcc::enable_clock(AhbPeripheral p) {
  unsigned bus = get_bus_index(p);
  unsigned slot = get_enable_index(p);

  switch (bus) {
    case 0: write_ahb1enr(read_ahb1enr().with_bit(slot, true)); break;
    case 1: write_ahb2enr(read_ahb2enr().with_bit(slot, true)); break;
    case 2: write_ahb3enr(read_ahb3enr().with_bit(slot, true)); break;
  }
  armv7m::data_synchronization_barrier();
}

void Rcc::disable_clock(AhbPeripheral p) {
  unsigned bus = get_bus_index(p);
  unsigned slot = get_enable_index(p);

  switch (bus) {
    case 0: write_ahb1enr(read_ahb1enr().with_bit(slot, false)); break;
    case 1: write_ahb2enr(read_ahb2enr().with_bit(slot, false)); break;
    case 2: write_ahb3enr(read_ahb3enr().with_bit(slot, false)); break;
  }
  armv7m::data_synchronization_barrier();
}

}  // namespace stm32f4xx
