#include "etl/assert.h"
#include "etl/scope_guard.h"

#include "etl/armv7m/implicit_crt0.h"

#include "vga/arena.h"
#include "vga/graphics_1.h"
#include "vga/rast/bitmap_1.h"
#include "vga/timing.h"
#include "vga/vga.h"

struct Demo {
  vga::rast::Bitmap_1 rasterizer{640, 480};
  vga::Band const band { &rasterizer, 480, nullptr };

  Demo() {
    rasterizer.set_fg_color(0b111111);
    rasterizer.set_bg_color(0b100000);

    if (!rasterizer.can_bg_use_bitband()) {
      rasterizer.flip();
      ETL_ASSERT(rasterizer.can_bg_use_bitband());
    }
  }
};

static void set_ball(vga::Graphics1 &g, unsigned x, unsigned y) {
  g.set_pixel(x, y);
  g.set_pixel(x - 1, y);
  g.set_pixel(x + 1, y);
  g.set_pixel(x, y - 1);
  g.set_pixel(x, y + 1);
}

static void clear_ball(vga::Graphics1 &g, unsigned x, unsigned y) {
  g.clear_pixel(x, y);
  g.clear_pixel(x - 1, y);
  g.clear_pixel(x + 1, y);
  g.clear_pixel(x, y - 1);
  g.clear_pixel(x, y + 1);
}

static void step_ball(vga::Graphics1 &g,
                      int &x, int &y,
                      int other_x, int other_y,
                      int &xi, int &yi) {
  clear_ball(g, x, y);
  x = other_x + xi;
  y = other_y + yi;

  if (x < 0) {
    x = 0;
    xi = -xi;
  }

  if (y < 0) {
    y = 0;
    yi = -yi;
  }

  if (x >= 640) {
    x = 639;
    xi = -xi;
  }

  if (y >= 480) {
    y = 479;
    yi = -yi;
  }

  set_ball(g, x, y);

  ++yi;
}

int main() {
  vga::init();
  vga::configure_timing(vga::timing_vesa_640x480_60hz);

  auto d = vga::arena_make<Demo>();

  vga::Graphics1 g = d->rasterizer.make_bg_graphics();
  g.clear_all();

  vga::configure_band_list(&d->band);
  ETL_ON_SCOPE_EXIT { vga::clear_band_list(); };

  vga::video_on();

  int x[2], y[2];
  int xi = 5, yi = 1;

  x[0] = x[1] = y[0] = y[1] = 0;

  while (true) {
    step_ball(g, x[0], y[0], x[1], y[1], xi, yi);
    d->rasterizer.copy_bg_to_fg();
    vga::sync_to_vblank();

    step_ball(g, x[1], y[1], x[0], y[0], xi, yi);
    d->rasterizer.copy_bg_to_fg();
    vga::sync_to_vblank();
  }
  __builtin_unreachable();
}
