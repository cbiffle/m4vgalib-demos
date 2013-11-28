#include "lib/armv7m/crt0.h"
#include "lib/armv7m/exception_table.h"
#include "lib/armv7m/instructions.h"
#include "lib/armv7m/scb.h"

#include "vga/vga.h"

void v7m_reset_handler() {
  armv7m::crt0_init();

  // Enable fault reporting.
  armv7m::scb.write_shcsr(armv7m::scb.read_shcsr()
                          .with_memfaultena(true)
                          .with_busfaultena(true)
                          .with_usgfaultena(true));

  // Enable floating point automatic/lazy state preservation.
  // The CONTROL bit governing FP will be set automatically when first used.
  armv7m::scb_fp.write_fpccr(armv7m::scb_fp.read_fpccr()
                             .with_aspen(true)
                             .with_lspen(true));
  armv7m::instruction_synchronization_barrier();  // Now please.

  // It is now safe to use floating point.

  vga::init();

  while (1);
}


/*
 * Catch unexpected exceptions in a loop.
 */
#define TRAP(name) \
  void v7m_ ## name ## _handler() { \
    while (1); \
  }

TRAP(nmi)
TRAP(hard_fault)
TRAP(mem_manage_fault)
TRAP(bus_fault)
TRAP(usage_fault)
TRAP(sv_call)
TRAP(debug_monitor)
TRAP(pend_sv)
TRAP(sys_tick)
