#include "lib/armv7m/exception_table.h"

#include "runtime/startup.h"

#include "vga/timing.h"
#include "vga/vga.h"
#include "vga/rast/direct.h"

#include <math.h>

using vga::rast::Direct;

static constexpr unsigned cols = 200;
static constexpr unsigned rows = 150;

static constexpr unsigned texture_width = 64;
static constexpr unsigned texture_height = 64;

static constexpr unsigned distance_frac_bits = 6;
static constexpr unsigned angle_frac_bits = 8;

static constexpr unsigned dfac = 1 << distance_frac_bits;
static constexpr unsigned afac = 1 << angle_frac_bits;

static constexpr unsigned dspeed = 5;
static constexpr unsigned aspeed = 5;

static constexpr float pi = 3.1415926f;

static Direct rasterizer(cols, rows);

static unsigned short distance[rows / 2 * cols / 2];
static unsigned short angle[rows / 2 * cols / 2];

static void generate_lookup_tables() {
  float r = 4;
  for (unsigned y = 0; y < rows/2; ++y) {
    float cy = y + 0.5f;
    for (unsigned x = 0; x < cols/2; ++x) {
      float cx = x + 0.5f;

      unsigned short d = r * texture_height / sqrtf(cx * cx + cy * cy) * dfac;
      unsigned short a =
          texture_width * 0.5f * (atan2f(cy, cx) / pi + 1) * afac;

      distance[y * cols/2 + x] = d;
      angle   [y * cols/2 + x] = a;
    }
  }
}

static unsigned char color(unsigned distance, unsigned d, unsigned a) {
  unsigned sel = (distance / (16*dfac));
  switch (sel) {
    case 0: return d^a;
    case 1: return (d^a) & 0b101010;
    case 2: return ((d^a) >> 1) & 0b01010101;
    case 3: return ((d^a) >> 1) & 0b01010101;
    default: return 0;
  }
}

static constexpr unsigned tex_period = texture_width;

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
        unsigned dx = cols/2 - x - 1;
        unsigned i = dy * cols/2 + dx;
        unsigned short d = (distance[i] + (frame << dspeed)) / dfac;
        unsigned short a = (-angle[i] + ((tex_period/2) << angle_frac_bits)
                              + (frame << aspeed)) / afac;
        fb[y * cols + x] = color(distance[i], d, a);
      }

      // Quadrant I
      for (unsigned x = cols/2; x < cols; ++x) {
        unsigned dx = x - cols/2;
        unsigned i = dy * cols/2 + dx;
        unsigned short d = (distance[i] + (frame << dspeed)) / dfac;
        unsigned short a = (angle[i] + (frame << aspeed)) / afac;
        fb[y * cols + x] = color(distance[i], d, a);
      }
    }

    // Quadrants III, IV
    for (unsigned y = rows/2; y < rows; ++y) {
      unsigned dy = y - rows/2;

      // Quadrant III
      for (unsigned x = 0; x < cols/2; ++x) {
        unsigned dx = cols/2 - x - 1;
        unsigned i = dy * cols/2 + dx;
        
        unsigned short d = (distance[i] + (frame << dspeed)) / dfac;
        unsigned short a = (angle[i] + ((tex_period/2) << angle_frac_bits)
                               + (frame << aspeed)) / afac;
        fb[y * cols + x] = color(distance[i], d, a);
      }

      // Quadrant IV
      for (unsigned x = cols/2; x < cols; ++x) {
        unsigned dx = x - cols/2;
        unsigned i = dy * cols/2 + dx;

        unsigned short d = (distance[i] + (frame << dspeed)) / dfac;
        unsigned short a = (-angle[i] + (tex_period << angle_frac_bits)
                               + (frame << aspeed)) / afac;
        fb[y * cols + x] = color(distance[i], d, a);
      }
    }

    rasterizer.flip();
  }
}

__attribute__((noreturn))
void v7m_reset_handler() {
  crt_init();
  rest();
}

