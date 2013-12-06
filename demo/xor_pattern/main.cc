#include "lib/armv7m/exception_table.h"

#include "runtime/startup.h"

#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/xor_pattern/xor.h"

static demo::xor_pattern::Xor rasterizer;

void v7m_reset_handler() {
  crt_init();
  vga::init();

  rasterizer.activate(vga::timing_vesa_800x600_60hz);
  vga::configure_band(0, 600, &rasterizer);
  vga::configure_timing(vga::timing_vesa_800x600_60hz);
  while (1);
}
