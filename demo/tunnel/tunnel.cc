#include "demo/tunnel/tunnel.h"

#include "etl/scope_guard.h"

#include "etl/armv7m/instructions.h"
#include "etl/armv7m/types.h"

#include "vga/arena.h"
#include "vga/measurement.h"
#include "vga/timing.h"
#include "vga/vga.h"
#include "vga/rast/direct_4.h"

#include "demo/input.h"
#include "demo/tunnel/config.h"
#include "demo/tunnel/table.h"

#define TABLE_IN_ROM 1

using vga::rast::Direct_4;

namespace demo {
namespace tunnel {

static unsigned tex_fetch(float u, float v) {
  return static_cast<unsigned>(u) ^ static_cast<unsigned>(v);
}

static unsigned shade(float distance, unsigned char pixel) {
  unsigned sel = unsigned(distance) / (config::texture_repeats_d * 2);
  sel = etl::armv7m::usat<3>(sel);

  return (pixel >> (0x01010000u >> (sel * 8)))
      & (0x5555AAFFu >> (sel * 8));
}

static unsigned color(float distance, float fd, float fa) {
  return shade(distance, tex_fetch(fd, fa));
}

struct Tunnel {
  vga::rast::Direct_4 rasterizer { config::cols, config::rows };
  vga::Band band { &rasterizer, config::rows * 4, nullptr };

#if TABLE_IN_ROM == 0
  table::Table tab;
#else
  table::Table tab{table::Table::compile_time_table};
#endif
};

void run() {
  vga::arena_reset();
  input_init();

  auto d = vga::arena_make<Tunnel>();

  bool video_on = false;
  ETL_ON_SCOPE_EXIT { if (video_on) vga::video_off(); };

  vga::configure_band_list(&d->band);
  ETL_ON_SCOPE_EXIT { vga::clear_band_list(); };

  auto & tab = d->tab;
  unsigned frame = 0;
  while (!user_button_pressed()) {
    unsigned char *fb = d->rasterizer.get_bg_buffer();
    ++frame;

    // Quadrants II, I
    for (unsigned y = 0; y < config::rows/2; ++y) {
      unsigned dy = config::rows/2 - y - 1;

      // Quadrant II
      for (unsigned x = 0; x < config::cols/2; ++x) {
        auto e = tab.get(config::cols/2 - x - 1, dy);
        float d = e.distance + frame*config::dspeed;
        float a = -e.angle + config::texture_period_a + frame*config::aspeed;
        fb[y * config::cols + x] = color(e.distance, d, a);
      }

      // Quadrant I
      for (unsigned x = config::cols/2; x < config::cols; ++x) {
        auto e = tab.get(x - config::cols/2, dy);
        float d = e.distance + frame*config::dspeed;
        float a = e.angle + frame*config::aspeed;
        fb[y * config::cols + x] = color(e.distance, d, a);
      }
    }

    // Quadrants III, IV
    for (unsigned y = config::rows/2; y < config::rows; ++y) {
      unsigned dy = y - config::rows/2;

      // Quadrant III
      for (unsigned x = 0; x < config::cols/2; ++x) {
        auto e = tab.get(config::cols/2 - x - 1, dy);
        float d = e.distance + frame*config::dspeed;
        float a = e.angle + frame*config::aspeed;
        fb[y * config::cols + x] = color(e.distance, d, a);
      }

      // Quadrant IV
      for (unsigned x = config::cols/2; x < config::cols; ++x) {
        auto e = tab.get(x - config::cols/2, dy);
        float d = e.distance + frame*config::dspeed;
        float a = -e.angle + config::texture_period_a + frame*config::aspeed;
        fb[y * config::cols + x] = color(e.distance, d, a);
      }
    }

    vga::msig_a_clear();
    d->rasterizer.flip();
    if (!video_on) {
      vga::video_on();
      video_on = true;
    }
    vga::msig_a_set();
  }
}

}  // namespace tunnel
}  // namespace demo
