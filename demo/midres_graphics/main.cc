#include "lib/armv7m/exception_table.h"

#include "runtime/startup.h"

#include "vga/graphics_1.h"
#include "vga/rast/bitmap_1.h"
#include "vga/timing.h"
#include "vga/vga.h"

static vga::rast::Bitmap_1 rasterizer(640, 480);

static vga::Band const band = { &rasterizer, 480, nullptr };

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

static void step_ball(int &x, int &y,
                      int other_x, int other_y,
                      int &xi, int &yi) {
  vga::Graphics1 g = rasterizer.make_bg_graphics();

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
  vga::sync_to_vblank();
  rasterizer.copy_bg_to_fg();
}

void v7m_reset_handler() {
  crt_init();
  vga::init();

  rasterizer.activate(vga::timing_vesa_800x600_60hz);
  vga::configure_band_list(&band);
  vga::configure_timing(vga::timing_vesa_640x480_60hz);

  rasterizer.set_fg_color(0b111111);
  rasterizer.set_bg_color(0b100000);

  if (!rasterizer.can_bg_use_bitband()) {
    rasterizer.flip();
    if (!rasterizer.can_bg_use_bitband()) while (1);
  }

  vga::video_on();

  int x[2], y[2];
  int xi = 5, yi = 1;

  x[0] = x[1] = y[0] = y[1] = 0;

  while (1) {
    step_ball(x[0], y[0], x[1], y[1], xi, yi);
    step_ball(x[1], y[1], x[0], y[0], xi, yi);
  }
}
