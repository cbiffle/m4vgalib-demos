#include "demo/tunnel/tunnel.h"

#include "etl/armv7m/instructions.h"

#include "vga/arena.h"
#include "vga/measurement.h"
#include "vga/timing.h"
#include "vga/vga.h"
#include "vga/rast/direct_4.h"

#include <math.h>

using vga::rast::Direct_4;

namespace demo {
namespace tunnel {

static constexpr unsigned cols = 200;
static constexpr unsigned rows = 150;

static constexpr unsigned texture_width = 64;
static constexpr unsigned texture_height = 64;

static constexpr float dspeed = 1.f;
static constexpr float aspeed = 0.2f;

static constexpr unsigned texture_repeats_d = 32;
static constexpr unsigned texture_repeats_a = 4;

static constexpr unsigned texture_period_a = texture_repeats_a * texture_width;
static constexpr unsigned texture_period_d = texture_repeats_d * texture_height;

static constexpr float pi = 3.1415926f;

static Direct_4 rasterizer(cols, rows);

static vga::Band const band = { &rasterizer, rows * 4, nullptr };

/*******************************************************************************
 * We use two lookup tables to eliminate transcendentals from the render loop.
 * Each table gives values for one quadrant of the screen; deriving values
 * for the other quadrants is straightforward.
 */

struct Entry {
  /*
   * Distance of each pixel in the quadrant from the near clip plane.  Modulo
   * texture_height, this gives the v coordinate.
   *
   * Range: [0, Infinity)
   */
  float distance;
  /*
   * Angle of each pixel in the quadrant from the Y axis.  This is given for
   * quadrant I, and can be flipped and offset for other quadrants.  Because we
   * use the angles as the texture u coordinate, we scale the natural range of
   * the trigonometric function used to a more convenient one.
   *
   * Range: [0, texture_width * texture_repeats_a)
   */
  float angle;
};

typedef unsigned PackedEntry;

static PackedEntry *table;

static Entry read_table(unsigned i) {
  PackedEntry const *address = &table[i];
  float distance, angle;
  asm (
    "  vldr.32 %[distance], [%[address]] \n"
    "  vcvtt.f32.f16 %[angle], %[distance] \n"
    "  vcvtb.f32.f16 %[distance], %[distance] \n"
  : [distance] "=&w" (distance),
    [angle]    "=&w" (angle)
  : [address] "r" (address)
  );
  return { distance, angle };
}

static void write_table(unsigned i, Entry entry) {
  PackedEntry const *address = &table[i];
  asm (
    "  vcvtb.f16.f32 %[distance], %[distance] \n"
    "  vcvtt.f16.f32 %[distance], %[angle] \n"
    "  vstr.32 %[distance], [%[address]] \n"
  :
  : [distance] "w" (entry.distance),
    [angle]    "w" (entry.angle),
    [address] "r" (address)
  );
}

static void generate_lookup_tables() {
  table = new PackedEntry[rows/2 * cols/2];
  for (unsigned y = 0; y < rows/2; ++y) {
    float cy = y + 0.5f;
    for (unsigned x = 0; x < cols/2; ++x) {
      float cx = x + 0.5f;

      unsigned i = y * cols/2 + x;
      float d = texture_period_d / sqrtf(cx * cx + cy * cy);
      float a = texture_period_a * 0.5f * (atan2f(cy, cx) / pi + 1);
      write_table(i, { d, a });
    }
  }
}

static unsigned tex_fetch(float u, float v) {
  return static_cast<unsigned>(u) ^ static_cast<unsigned>(v);
}

static unsigned shade(float distance, unsigned char pixel) {
  unsigned sel = static_cast<unsigned>(distance) / (texture_repeats_d * 2);
  sel = etl::armv7m::usat<3>(sel);

  return (pixel >> (0x01010000u >> (sel * 8)))
      & (0x5555AAFFu >> (sel * 8));
}

static unsigned color(float distance, float fd, float fa) {
  return shade(distance, tex_fetch(fd, fa));
}

void run(unsigned frame_count) {
  generate_lookup_tables();

  rasterizer.activate(vga::timing_vesa_800x600_60hz);
  vga::configure_band_list(&band);

  bool video_on = false;

  unsigned frame = 0;
  while (frame_count == 0 || frame < frame_count) {
    unsigned char *fb = rasterizer.get_bg_buffer();
    ++frame;

    // Quadrants II, I
    for (unsigned y = 0; y < rows/2; ++y) {
      unsigned dy = rows/2 - y - 1;

      // Quadrant II
      for (unsigned x = 0; x < cols/2; ++x) {
        Entry e = read_table(dy * cols/2 + (cols/2 - x - 1));
        float d = e.distance + frame*dspeed;
        float a = -e.angle + texture_period_a + frame*aspeed;
        fb[y * cols + x] = color(e.distance, d, a);
      }

      // Quadrant I
      for (unsigned x = cols/2; x < cols; ++x) {
        Entry e = read_table(dy * cols/2 + (x - cols/2));
        float d = e.distance + frame*dspeed;
        float a = e.angle + frame*aspeed;
        fb[y * cols + x] = color(e.distance, d, a);
      }
    }

    // Quadrants III, IV
    for (unsigned y = rows/2; y < rows; ++y) {
      unsigned dy = y - rows/2;

      // Quadrant III
      for (unsigned x = 0; x < cols/2; ++x) {
        Entry e = read_table(dy * cols/2 + (cols/2 - x - 1));
        float d = e.distance + frame*dspeed;
        float a = e.angle + frame*aspeed;
        fb[y * cols + x] = color(e.distance, d, a);
      }

      // Quadrant IV
      for (unsigned x = cols/2; x < cols; ++x) {
        Entry e = read_table(dy * cols/2 + (x - cols/2));
        float d = e.distance + frame*dspeed;
        float a = -e.angle + texture_period_a + frame*aspeed;
        fb[y * cols + x] = color(e.distance, d, a);
      }
    }

    vga::msig_a_clear();
    rasterizer.flip();
    if (!video_on) {
      vga::video_on();
      video_on = true;
    }
    vga::msig_a_set();
  }

  vga::video_off();
  vga::configure_band_list(nullptr);
  vga::arena_reset();
}

}  // namespace tunnel
}  // namespace demo
