#include "lib/armv7m/exception_table.h"

#include "runtime/startup.h"

#include "vga/rast/bitmap_1.h"
#include "vga/graphics_1.h"
#include "vga/measurement.h"
#include "vga/timing.h"
#include "vga/vga.h"
#include "demo/conway/conway.h"

static constexpr unsigned conway_cols = 800;
static constexpr unsigned conway_rows = 600;

static vga::rast::Bitmap_1 rasterizer(conway_cols, conway_rows);

// Cells will be set at boot when rand() is less than this.
static unsigned constexpr set_threshold = 0x20000000;

// Seed and state variable for random number generation.
static unsigned seed = 1118;

// A simple linear congruential random number generator.
// (Coefficients borrowed from GCC.)
static unsigned rand() {
  seed = ((seed * 1103515245) + 12345) & 0x7FFFFFFF;
  return seed;
}

static void set_random_cells() {
  vga::Graphics1 g = rasterizer.make_bg_graphics();
  for (unsigned y = 0; y < conway_rows; ++y) {
    for (unsigned x = 0; x < conway_cols; ++x) {
      if (rand() < set_threshold) g.set_pixel(x, y);
    }
  }
  rasterizer.flip();
}

void v7m_reset_handler() {
  crt_init();
  vga::init();

  rasterizer.activate(vga::timing_vesa_800x600_60hz);
  rasterizer.set_fg_color(0b111111);
  rasterizer.set_bg_color(0b010000);

  vga::configure_band(0, conway_rows, &rasterizer);
  vga::configure_timing(vga::timing_vesa_800x600_60hz);

  set_random_cells();

  while (true) {
    step(static_cast<unsigned const *>(rasterizer.get_fg_buffer()),
         static_cast<unsigned *>(rasterizer.get_bg_buffer()),
         conway_cols / 32,
         conway_rows);

    vga::msig_a_clear();
    // We often complete the update inside of vblank, so we don't use flip(),
    // which syncs to the *start* of the vblank.
    vga::wait_for_vblank();
    rasterizer.flip_now();
    vga::msig_a_set();
  }
}
