#include "etl/armv7m/exception_table.h"

#include "etl/armv7m/crt0.h"

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
static vga::Band const band = { &rasterizer, 600, nullptr };

void etl_armv7m_reset_handler() {
  etl::armv7m::crt0_init();
  vga::init();

  rasterizer.activate(vga::timing_vesa_800x600_60hz);
  vga::configure_band_list(&band);
  vga::configure_timing(vga::timing_vesa_800x600_60hz);
  vga::video_on();
  while (1);
}
