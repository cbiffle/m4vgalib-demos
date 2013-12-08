#include "lib/armv7m/exception_table.h"
#include "lib/armv7m/instructions.h"

#include "runtime/startup.h"

#include "vga/measurement.h"
#include "vga/timing.h"
#include "vga/vga.h"
#include "vga/rast/direct.h"

#include <math.h>

using vga::rast::Direct;

static constexpr unsigned cols = 200;
static constexpr unsigned rows = 150;

static constexpr unsigned texture_width = 64;
static constexpr unsigned texture_height = 64;

static constexpr float dspeed = 1.f;
static constexpr float aspeed = 0.2f;

static constexpr unsigned dmult = 2;
static constexpr unsigned amult = 2;

static constexpr unsigned texture_repeats_d = 32;
static constexpr unsigned texture_repeats_a = 2;

static constexpr unsigned texture_period_a = texture_repeats_a * texture_width;
static constexpr unsigned texture_period_d = texture_repeats_d * texture_height;

static constexpr float pi = 3.1415926f;

static Direct rasterizer(cols, rows);


/*******************************************************************************
 * We use two lookup tables to eliminate transcendentals from the render loop.
 * Each table gives values for one quadrant of the screen; deriving values
 * for the other quadrants is straightforward.
 */

/*
 * Distance of each pixel in the quadrant from the near clip plane.  Modulo
 * texture_height, this gives the v coordinate.
 *
 * Range: [0, Infinity)
 */
static float distance[rows / 2 * cols / 2];

/*
 * Angle of each pixel in the quadrant from the Y axis.  This is given for
 * quadrant I, and can be flipped and offset for other quadrants.  Because we
 * use the angles as the texture u coordinate, we scale the natural range of
 * the trigonometric function used to a more convenient one.
 *
 * Range: [0, texture_width * texture_repeats_a)
 */
static float angle[rows / 2 * cols / 2];

static void generate_lookup_tables() {
  for (unsigned y = 0; y < rows/2; ++y) {
    float cy = y + 0.5f;
    for (unsigned x = 0; x < cols/2; ++x) {
      float cx = x + 0.5f;

      unsigned i = y * cols/2 + x;
      distance[i] = texture_period_d / sqrtf(cx * cx + cy * cy);
      angle[i] = texture_period_a * 0.5f * (atan2f(cy, cx) / pi + 1);
    }
  }
}

static unsigned tex_fetch(float u, float v) {
  return static_cast<unsigned>(u * dmult) ^ static_cast<unsigned>(v * amult);
}

static unsigned shade(float distance, unsigned char pixel) {
  unsigned sel = static_cast<unsigned>(distance) / (texture_repeats_d * 4);
  sel = armv7m::usat<3>(sel);

  return (pixel >> (0x01010000u >> (sel * 8)))
      & (0x5555AAFFu >> (sel * 8));
}

static unsigned color(float distance, float fd, float fa) {
  return shade(distance, tex_fetch(fd, fa));
}

__attribute__((noreturn))
__attribute__((noinline))
static void rest() {
  vga::init();

  generate_lookup_tables();

  rasterizer.activate(vga::timing_vesa_800x600_60hz);
  vga::configure_band(0, rows * 4, &rasterizer);
  vga::configure_timing(vga::timing_vesa_800x600_60hz);

  unsigned frame = 0;
  while (1) {
    unsigned char *fb = rasterizer.get_bg_buffer();
    ++frame;

    // Quadrants II, I
    for (unsigned y = 0; y < rows/2; ++y) {
      unsigned dy = rows/2 - y - 1;

      // Quadrant II
      for (unsigned x = 0; x < cols/2; ++x) {
        unsigned i = dy * cols/2 + (cols/2 - x - 1);
        float d = distance[i] + frame*dspeed;
        float a = -angle[i] + texture_period_a + frame*aspeed;
        fb[y * cols + x] = color(distance[i], d, a);
      }

      // Quadrant I
      for (unsigned x = cols/2; x < cols; ++x) {
        unsigned i = dy * cols/2 + (x - cols/2);
        float d = distance[i] + frame*dspeed;
        float a = angle[i] + frame*aspeed;
        fb[y * cols + x] = color(distance[i], d, a);
      }
    }

    // Quadrants III, IV
    for (unsigned y = rows/2; y < rows; ++y) {
      unsigned dy = y - rows/2;

      // Quadrant III
      for (unsigned x = 0; x < cols/2; ++x) {
        unsigned i = dy * cols/2 + (cols/2 - x - 1);
        float d = distance[i] + frame*dspeed;
        float a = angle[i] + frame*aspeed;
        fb[y * cols + x] = color(distance[i], d, a);
      }

      // Quadrant IV
      for (unsigned x = cols/2; x < cols; ++x) {
        unsigned i = dy * cols/2 + (x - cols/2);
        float d = distance[i] + frame*dspeed;
        float a = -angle[i] + texture_period_a + frame*aspeed;
        fb[y * cols + x] = color(distance[i], d, a);
      }
    }

    vga::msig_a_clear();
    rasterizer.flip();
    vga::msig_a_set();
  }
}

__attribute__((noreturn))
void v7m_reset_handler() {
  crt_init();
  rest();
}
