#include "lib/armv7m/exception_table.h"

#include "runtime/startup.h"

#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/xor_pattern/xor.h"

static demo::xor_pattern::Xor rasterizer;
static vga::Band const band = { &rasterizer, 600, nullptr };

void v7m_reset_handler() {
  crt_init();
  vga::init();

  rasterizer.activate(vga::timing_vesa_800x600_60hz);
  vga::configure_band_list(&band);
  vga::configure_timing(vga::timing_vesa_800x600_60hz);
  vga::video_on();
  while (1);
}
