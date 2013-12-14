#include "lib/armv7m/exception_table.h"

#include "runtime/startup.h"

#include "vga/bitmap.h"
#include "vga/font_10x16.h"
#include "vga/graphics_1.h"
#include "vga/timing.h"
#include "vga/measurement.h"
#include "vga/vga.h"
#include "vga/rast/bitmap_1.h"

#include <math.h>

static vga::rast::Bitmap_1 rasterizer(800, 600);

static vga::Band const band = { &rasterizer, 600, nullptr };

__attribute__((noreturn))
__attribute__((noinline))
static void rest() {
  crt_init();
  vga::init();

  rasterizer.activate(vga::timing_vesa_800x600_60hz);
  rasterizer.set_fg_color(0b111111);
  rasterizer.set_bg_color(0b010000);

  rasterizer.make_bg_graphics().clear_all();

  vga::configure_band_list(&band);
  vga::configure_timing(vga::timing_vesa_800x600_60hz);

  /*
  if (!rasterizer.can_bg_use_bitband()) {
    rasterizer.flip();
    if (!rasterizer.can_bg_use_bitband()) while (1);
  }
  */
  unsigned n = 256 - ' ';
  vga::Bitmap b = { (unsigned *) (void *) &vga::font_10x16[' '],
                    n * 8,
                    16,
                    256/4 };

  vga::Bitmap d = rasterizer.get_bg_bitmap();

  // Word-aligned copy
  vga::bitblt(b, 0, 0, d, { 0, 0, 700, 16 });

  // Similarly aligned copy
  vga::bitblt(b, 8, 0, d, { 8, 16, 700, 16});
  
  // Source-aligned copy
  vga::bitblt(b, 0, 0, d, { 8, 32, 700, 16});

  // Destination-aligned copy
  vga::bitblt(b, 8, 0, d, { 0, 48, 700, 16});

  // Unaligned copy
  vga::bitblt(b, 8, 0, d, { 4, 64, 700, 16});

  rasterizer.flip_now();
  vga::video_on();

  while (1);  
}

__attribute__((noreturn))
void v7m_reset_handler() {
  crt_init();
  rest();
}
