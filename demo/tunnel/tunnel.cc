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

using vga::rast::Direct_4;

namespace demo {
namespace tunnel {

/*
 * Can't decide if I like these two features or not, so I'm leaving the code in.
 * Edit the constant to adjust.
 */
enum class Effect {
  none,

  /*
   * Make pixels darker as distance increases.  Because we really only have four
   * luminance levels in the current system, it winds up kind of banded without
   * dithering (below).
   *
   * Cost: ~59% slower rendering (2.8ms/frame vs. 1.76ms)
   */
  depth_shading,

  /*
   * Dither darkness bands, both spatially and temporally.  This helps to hide
   * the band edges.
   *
   * Cost: ~57% slower rendering (4.4ms/frame vs. 2.8ms)
   */
  dithering,
};

static constexpr Effect effect = Effect::none;


/*
 * Texture "lookup" generates the traditional procedural texture.
 */
static uint_fast8_t tex_fetch(float u, float v) {
  return uint_fast8_t(u) ^ uint_fast8_t(v);
}

/*
 * The shader applies a depth effect and (optionally) dithering.
 */
static uint_fast8_t shade(float distance,
                          uint_fast8_t pixel,
                          bool dither) {
  unsigned sel = effect != Effect::dithering
    ? unsigned(distance / (config::texture_repeats_d * 2))
    : dither
      ? unsigned(distance / (config::texture_repeats_d * 1.5f))
      : unsigned(distance / (config::texture_repeats_d * 4.f));
  sel = etl::armv7m::usat<3>(sel);

  return (pixel >> (0x01010000u >> (sel * 8)))
      & (0x5555AAFFu >> (sel * 8));
}

/*
 * Facade for shade and tex_fetch.
 */
static uint_fast8_t color(float distance,
                          float fd,
                          float fa,
                          bool dither) {
  return effect != Effect::none
    ? shade(distance, tex_fetch(fd, fa), dither)
    : tex_fetch(fd, fa);
}


/*
 * Demo state.
 */
struct Tunnel {
  vga::rast::Direct_4 rasterizer { config::cols, config::rows };
  vga::Band band { &rasterizer, config::rows * 4, nullptr };

#if TABLE_IN_ROM == 0
  table::Table tab;
#else
  table::Table const & tab = table::Table::compile_time_table;
#endif
};


/*
 * Entry point.
 */
void run() {
  vga::arena_reset();
  vga::msigs_init();
  input_init();

  auto d = vga::arena_make<Tunnel>();

  bool video_on = false;
  ETL_ON_SCOPE_EXIT { if (video_on) vga::video_off(); };

  vga::configure_band_list(&d->band);
  ETL_ON_SCOPE_EXIT { vga::clear_band_list(); };

  auto & tab = d->tab;
  unsigned frame = 0;
  bool dither = false;
  while (!user_button_pressed()) {
    uint8_t *fb = d->rasterizer.get_bg_buffer();
    ++frame;

    for (unsigned y = 0; y < config::rows/2; ++y) {
      vga::msig_b_toggle();
      for (unsigned x = 0; x < config::cols/2; ++x) {
        auto e = tab.get(x, y);
        float d = e.distance + frame*config::dspeed;

        float a1 = -e.angle + config::texture_period_a + frame*config::aspeed;
        auto p1 = color(e.distance, d, a1, dither);

        // Quadrant IV
        fb[(y + config::rows/2) * config::cols + x + config::cols/2] = p1;

        // Quadrant II
        fb[(config::rows/2 - 1 - y) * config::cols
           + (config::cols/2 - 1 - x)] = p1;

        float a2 = e.angle + frame*config::aspeed;
        auto p2 = color(e.distance, d, a2, !dither);

        // Quadrant I
        fb[(config::rows/2 - 1 - y) * config::cols
           + x + config::cols/2] = p2;

        // Quadrant III
        fb[(y + config::rows/2) * config::cols
           + (config::cols/2 - 1 - x)] = p2;

        dither = !dither;
      }
      dither = !dither;
    }

    // Temporal dithering:
    dither ^= (frame & 1);

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
