#include "demo/tunnel/tunnel.h"

#include "etl/scope_guard.h"

#include "etl/armv7m/instructions.h"
#include "etl/armv7m/types.h"

#include "vga/arena.h"
#include "vga/measurement.h"
#include "vga/timing.h"
#include "vga/vga.h"
#include "vga/rast/direct_2.h"
#include "vga/rast/direct_2_mirror.h"

#include "demo/input.h"
#include "demo/tunnel/config.h"
#include "demo/tunnel/table.h"

using vga::rast::Direct_2;
using vga::rast::Direct_2_Mirror;

using demo::tunnel::table::Entry;

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
};

static constexpr Effect effect = Effect::none;


/*
 * Texture "lookup" generates the traditional procedural texture.
 */
static uint_fast8_t tex_fetch(float u, float v) {
  return uint_fast8_t(u) ^ uint_fast8_t(v);
}

/*
 * The shader applies a depth effect.
 */
static uint_fast8_t shade(float distance,
                          uint_fast8_t pixel) {
  unsigned sel = unsigned(distance / (config::texture_repeats_d * 2));
  sel = etl::armv7m::usat<3>(sel);

  return (pixel >> (0x01010000u >> (sel * 8)))
       & (0x5555AAFFu >> (sel * 8));
}

/*
 * Facade for shade and tex_fetch.
 *
 * 'distance' is distance from the viewer, whereas 'fd' is the Z position
 * that advances as time passes.
 */
static uint_fast8_t color(float distance,
                          float fd,
                          float fa) {
  return shade(distance, tex_fetch(fd, fa));
}


/*
 * Demo state.
 */
struct Tunnel {
  vga::rast::Direct_2 rast1 { config::cols, config::rows/2 };
  vga::rast::Direct_2_Mirror rast2 { rast1, config::rows };

  vga::Band bands[2] {
    { &rast1, config::rows, &bands[1] },
    { &rast2, config::rows, nullptr },
  };

  table::Table const & tab = table::Table::compile_time_table;

  void inner_render_loop(uint8_t * fb, unsigned frame);
};

__attribute__((optimize("prefetch-loop-arrays")))
void Tunnel::inner_render_loop(uint8_t * fb,
                               unsigned frame) {
  float z = frame * config::dspeed;
  float a = frame * config::aspeed;

  for (unsigned y = 0; y < config::quad_height / config::sub; ++y) {
    auto top_left = tab.get(0, y);
    auto bot_left = tab.get(0, y + 1);

    for (unsigned x = 0; x < config::quad_width / config::sub; ++x) {
      auto top_right = tab.get(x + 1, y);
      auto bot_right = tab.get(x + 1, y + 1);

      Entry left { top_left.distance, top_left.angle };
      Entry const left_i {
        (bot_left.distance - top_left.distance) / config::sub,
        (bot_left.angle - top_left.angle) / config::sub,
      };

      Entry right { top_right.distance, top_right.angle };
      Entry const right_i {
        (bot_right.distance - top_right.distance) / config::sub,
        (bot_right.angle - top_right.angle) / config::sub,
      };

      for (unsigned sy = y * config::sub; sy < (y + 1) * config::sub; ++sy) {
        auto inv_sy = config::quad_height - 1 - sy;

        Entry v { left.distance, left.angle };
        Entry i { (right.distance - left.distance) / config::sub,
                  (right.angle - left.angle) / config::sub };

        for (unsigned sx = x * config::sub; sx < (x + 1) * config::sub; ++sx) {
          float a1 = -v.angle + config::texture_period_a + a;
          auto p1 = color(v.distance, a1, v.distance + z);

          // Quadrant II
          fb[inv_sy * config::cols + (config::cols/2 - 1 - sx)] = p1;

          float a2 = v.angle + a;
          auto p2 = color(v.distance, a2, v.distance + z);

          // Quadrant I
          fb[inv_sy * config::cols + sx + config::cols/2] = p2;

          v = { v.distance + i.distance,
                v.angle + i.angle };
        }

        left = { left.distance + left_i.distance,
                 left.angle + left_i.angle };
        right = { right.distance + right_i.distance,
                  right.angle + right_i.angle };
      }

      top_left = top_right;
      bot_left = bot_right;
    }
  }
}


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

  vga::configure_band_list(d->bands);
  ETL_ON_SCOPE_EXIT { vga::clear_band_list(); };

  unsigned frame = 0;
  while (!user_button_pressed()) {
    uint8_t *fb = d->rast1.get_bg_buffer();
    ++frame;

    d->inner_render_loop(fb, frame);

    vga::msig_a_clear();
    vga::sync_to_vblank();
    d->rast1.flip_now();
    if (!video_on) {
      vga::video_on();
      video_on = true;
    }
    vga::msig_a_set();
  }
}

}  // namespace tunnel
}  // namespace demo
