#include "lib/armv7m/crt0.h"
#include "lib/armv7m/exception_table.h"
#include "lib/armv7m/instructions.h"
#include "lib/armv7m/scb.h"

#include "runtime/ramcode.h"

#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/xor_pattern/xor.h"

static demo::xor_pattern::Xor rasterizer;

void v7m_reset_handler() {
  armv7m::crt0_init();
  runtime::ramcode_init();

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

  // Enable access to the floating point coprocessor.
  armv7m::scb.write_cpacr(armv7m::scb.read_cpacr()
                          .with_cp11(armv7m::Scb::CpAccess::full)
                          .with_cp10(armv7m::Scb::CpAccess::full));

  // It is now safe to use floating point.

  vga::init();

  rasterizer.activate(vga::timing_vesa_800x600_60hz);
  vga::configure_band(0, 600, &rasterizer);
  vga::configure_timing(vga::timing_vesa_800x600_60hz);
  while (1);
}
