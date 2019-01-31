#include "demo/tunnel/tunnel.h"

#include "etl/scope_guard.h"
#include "etl/math/saturation.h"

#include "vga/arena.h"
#include "vga/measurement.h"
#include "vga/timing.h"
#include "vga/vga.h"
#include "vga/rast/direct.h"
#include "vga/rast/direct_mirror.h"

#include "demo/input.h"
#include "demo/tunnel/config.h"
#include "demo/tunnel/table.h"

using demo::tunnel::table::Entry;

namespace demo {
namespace tunnel {

/*
 * Texture "lookup" generates the traditional procedural texture.
 */
ETL_INLINE
static uint_fast8_t tex_fetch(float u, float v) {
  return uint_fast8_t(u) ^ uint_fast8_t(v);
}

/*
 * The shader applies a depth effect.
 */
ETL_INLINE
static uint_fast8_t shade(float distance,
                          uint_fast8_t pixel) {
  unsigned sel = unsigned(distance / (config::texture_repeats_d * 2));
  // Clamp value as signed signed to trigger GCC's usat matching pattern (bug).
  sel = unsigned(etl::math::clamp(int(sel), 0, 7));

  return (pixel >> (0x01010000u >> (sel * 8)))
       & (0x5555AAFFu >> (sel * 8));
}

/*
 * Facade for shade and tex_fetch.
 *
 * 'distance' is distance from the viewer, whereas 'fd' is the Z position
 * that advances as time passes.
 */
ETL_INLINE
static uint_fast8_t color(float distance,
                          float fd,
                          float fa) {
  return shade(distance, tex_fetch(fd, fa));
}


/*
 * Demo state.
 */
struct Tunnel {
  vga::rast::Direct rast1 {
    config::cols * 2, config::rows,
    2, 2,
  };
  vga::rast::DirectMirror rast2 { rast1, config::rows };

  vga::Band bands[2] {
    { &rast1, config::rows, &bands[1] },
    { &rast2, config::rows, nullptr },
  };

  table::Table const & tab = table::Table::compile_time_table();

  void inner_render_loop(uint8_t * fb, unsigned frame);
};

/*
 * As discussed in table.h, this works by upsampling a lookup table through
 * bilinear interpolation and some trig identities.
 *
 * This implementation, specifically, is designed to:
 *
 * 1. Maximize Flash cache hit rate by accessing the lookup table in a strict
 *    linear fashion, while ignoring RAM locality.
 *
 * 2. Reduce, as much as practical, the number of lookup table loads by
 *    carrying values in the plentiful FPU registers.
 *
 * 3. Minimize the number of divisions by non-constant values.
 */
__attribute__((optimize("prefetch-loop-arrays")))
void Tunnel::inner_render_loop(uint8_t * fb,
                               unsigned frame) {
  // The distance we have traveled into the tunnel.
  float z = frame * config::dspeed;
  // The angle of the tunnel's rotation.
  float a = frame * config::aspeed;
  // TODO: both of these values tend to wind up and lose precision if you let
  // the demo run, say, overnight.  Since they're both fundamentally periodic
  // we could fix this by wrapping.

  // Outer loops: iterate over each macroblock in the display, left-to-right,
  // top-to-bottom.  'y' and 'x' are in macroblock (table) coordinates.
  for (unsigned y = 0; y < config::quad_height / config::sub; ++y) {
    // To process a macroblock, we need to look up the table entries at each of
    // its four corners.  When processing macroblocks left to right, the right
    // corners of a block are the left corners of its neighbor -- so we can save
    // table lookups by "shifting" the entries across.

    // Bootstrap the process by loading the leftmost two corners for this row.
    auto top_left = tab.get(0, y);
    auto bot_left = tab.get(0, y + 1);

    for (unsigned x = 0; x < config::quad_width / config::sub; ++x) {
      // Load the two corners at the right side of the current block.
      auto const top_right = tab.get(x + 1, y);
      auto const bot_right = tab.get(x + 1, y + 1);

      // And now we fire up a stepwise bilinear interpolator in both distance
      // and angle.  To interpolate the table entry for a pixel in the
      // macroblock, we first linearly interpolate the values along the left
      // and right edges at its Y coordinate, and then interpolate between them
      // at its X coordinate.
      //
      // We do this stepwise by calculating the linear equation of both distance
      // and angle on both the left and right sides, given as a value and a
      // slope, or increment: (left, left_i) and (right, right_i).  We'll update
      // the position in-place, but the slopes are constant.
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

      // Process pixel rows within the macroblock.  'sy' and 'sx' are in
      // pixel coordinates.
      for (unsigned sy = y * config::sub; sy < (y + 1) * config::sub; ++sy) {
        // We'll need this term repeatedly below; precompute it.
        auto const inv_sy = config::quad_height - 1 - sy;

        // Fire up the second dimension of the bilinear interpolator, this time
        // moving from the value of 'left' to the value of 'right'.
        Entry v { left.distance, left.angle };
        Entry const i { (right.distance - left.distance) / config::sub,
                        (right.angle - left.angle) / config::sub };

        for (unsigned sx = x * config::sub; sx < (x + 1) * config::sub; ++sx) {
          // Quadrant II (upper-left): apply trig identity to correct the angle
          // value.
          auto const a1 = -v.angle + config::texture_period_a + a;
          auto const p1 = color(v.distance, a1, v.distance + z);
          fb[inv_sy * config::cols + (config::cols/2 - 1 - sx)] = p1;

          // Quadrant I (upper-right): use the angle value as written.
          auto const a2 = v.angle + a;
          auto const p2 = color(v.distance, a2, v.distance + z);
          fb[inv_sy * config::cols + sx + config::cols/2] = p2;

          // Quadrants III/IV, of course, are handled through rasterization
          // tricks, and not computed here.

          // Advance the horizontal linear interpolator toward 'right'.
          v = { v.distance + i.distance,
                v.angle + i.angle };
        }

        // Advance the vertical linear interpolators toward 'bot_left' and
        // 'bot_right', respectively.
        left = { left.distance + left_i.distance,
                 left.angle + left_i.angle };
        right = { right.distance + right_i.distance,
                  right.angle + right_i.angle };
      }

      // Shift the right corners to become the new left corners.
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
