#include "lib/armv7m/exception_table.h"

#include "runtime/startup.h"

#include "vga/rasterizer.h"
#include "vga/timing.h"
#include "vga/vga.h"

class Nothing : public vga::Rasterizer {
public:
  __attribute__((section(".ramcode")))
  LineShape rasterize(unsigned, Pixel *out) {
    for (unsigned i = 0; i < 800; i += 2) {
      out[i] = 0xFF;
      out[i + 1] = 0x00;
    }
    return { 0, 800 };
  }
};

static Nothing rasterizer;

void v7m_reset_handler() {
  crt_init();
  vga::init();

  rasterizer.activate(vga::timing_vesa_800x600_60hz);
  vga::configure_band(0, 600, &rasterizer);
  vga::configure_timing(vga::timing_vesa_800x600_60hz);
  vga::video_on();
  while (1);
}
