#include "etl/armv7m/exception_table.h"

#include "etl/armv7m/crt0.h"

#include "vga/arena.h"
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

struct Demo {
  Nothing rasterizer;
  vga::Band const band[2]{
    {&rasterizer, 1, &band[1]},
    {nullptr, 599, nullptr},
  };
};

void etl_armv7m_reset_handler() {
  etl::armv7m::crt0_init();
  vga::init();

  auto d = vga::arena_make<Demo>();

  vga::configure_band_list(d->band);
  vga::configure_timing(vga::timing_vesa_800x600_60hz);
  vga::video_on();
  while (1);
}
