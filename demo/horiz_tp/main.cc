#include "etl/armv7m/implicit_crt0.h"
#include "etl/attribute_macros.h"

#include "vga/arena.h"
#include "vga/rasterizer.h"
#include "vga/timing.h"
#include "vga/vga.h"

class Nothing : public vga::Rasterizer {
public:
  ETL_SECTION(".ramcode")
  RasterInfo rasterize(unsigned cpp, unsigned, Pixel *out) override {
    for (unsigned i = 0; i < 800; i += 2) {
      out[i] = 0xFF;
      out[i + 1] = 0x00;
    }
    return {
      .offset = 0,
      .length = 800,
      .cycles_per_pixel = cpp,
      .repeat_lines = 599,
    };
  }
};

struct Demo {
  Nothing rasterizer;
  vga::Band const band[1]{
    {&rasterizer, 600, nullptr},
  };
};

int main() {
  vga::init();

  auto d = vga::arena_make<Demo>();

  vga::configure_band_list(d->band);
  vga::configure_timing(vga::timing_vesa_800x600_60hz);
  vga::video_on();
  while (true);
  __builtin_unreachable();
}
