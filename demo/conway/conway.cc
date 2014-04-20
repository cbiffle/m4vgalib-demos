#include "demo/conway/conway.h"

#include "etl/attribute_macros.h"

#include "vga/rast/bitmap_1.h"
#include "vga/arena.h"
#include "vga/graphics_1.h"
#include "vga/measurement.h"
#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/input.h"

namespace demo {
namespace conway {

/*
 * Result of a bit-parallel addition operation: a pair of bit vectors
 * representing sum and carry.
 */
struct AddResult {
  unsigned sum;
  unsigned carry;
};

/*
 * Bit-parallel half adder: adds corresponding bits of two 32-bit vectors,
 * producing sum and carry vectors.
 */
static ETL_INLINE AddResult half_add(unsigned a, unsigned b) {
  return { a ^ b, a & b };
}

/*
 * Bit-parallel full adder: add corresponding bits of *three* 32-bit vectors,
 * producing sum and carry vectors.
 */
static ETL_INLINE AddResult full_add(unsigned a, unsigned b, unsigned c) {
  AddResult r0 = half_add(a, b);
  AddResult r1 = half_add(r0.sum, c);
  return { r1.sum, r0.carry | r1.carry };
}

/*
 * Step the automaton for the 32 cells contained in current[1], using the
 * neighboring bit vectors for context.
 */
__attribute__((section(".ramcode")))
static ETL_INLINE unsigned col_step(unsigned above[3],
                                unsigned current[3],
                                unsigned below[3]) {
  /*
   * Compute row-wise influence sums.  This produces 96 2-bit sums (represented
   * as three pairs of 32-vectors) giving the number of live cells in the 1D
   * Moore neighborhood around each position.
   */
  AddResult a_inf = full_add((above[1] << 1) | (above[0] >> 31),
                             above[1],
                             (above[1] >> 1) | (above[2] << 31));
  AddResult c_inf = half_add((current[1] << 1) | (current[0] >> 31),
                             /* middle bits of current[1] don't count */
                             (current[1] >> 1) | (current[2] << 31));
  AddResult b_inf = full_add((below[1] << 1) | (below[0] >> 31),
                             below[1],
                             (below[1] >> 1) | (below[2] << 31));

  /*
   * Sum the row-wise sums into a two-dimensional Moore neighborhood population
   * count.  Such a count can overflow into four bits, but we don't care: Conway
   * has the same result for 8/9 and 0/1 (the cell is cleared in both cases).
   *
   * Thus, we don't need a four-bit addition.  Instead, we just retain the
   * carry output from the two intermediate additions and use it as a mask.
   */
  AddResult next0 = full_add(a_inf.sum, c_inf.sum, b_inf.sum);
  AddResult next1a = full_add(a_inf.carry, next0.carry, b_inf.carry);
  AddResult next1b = half_add(c_inf.carry, next1a.sum);

  /*
   * Apply Niemiec's optimization: OR the current cell state vector into the
   * 9-cell neighborhoold population count to derive the new state cheaply.  The
   * cell is set iff its three-bit sum is 0b011.
   */
  return (next0.sum | current[1])
       & next1b.sum
       & ~next1a.carry
       & ~next1b.carry;
}

/*
 * Advance the automaton.
 *  - current_map is the framebuffer (or equivalent bitmap) holding the current
 *    state.
 *  - next_map is a framebuffer (bitmap) that will be filled in.
 *  - width is the width of both buffers in words.
 *  - height is the height of both buffers in lines.
 */
static void step(unsigned const *current_map,
          unsigned *next_map,
          unsigned width,
          unsigned height);

#define ADV(name, next) \
  name[0] = name[1]; \
  name[1] = name[2]; \
  name[2] = (next)

__attribute__((section(".ramcode")))
void step(unsigned const *current_map,
          unsigned *next_map,
          unsigned width,
          unsigned height) {
  // We keep sliding windows of state in these arrays.
  unsigned above[3] = { 0, 0, 0 };
  unsigned current[3] = { 0, 0, 0 };
  unsigned below[3] = { 0, 0, 0 };

  // Bootstrap for first column of first row.
  current[0] = current[1] = 0;
  current[2] = current_map[0];

  below[0] = below[1] = 0;
  below[2] = current_map[width];

  // First row, wherein above[x] = 0, less final column
  for (unsigned x = 0; x < width - 1; ++x) {
    ADV(current, current_map[x + 1]);
    ADV(below,   current_map[width + x + 1]);
    next_map[x] = col_step(above, current, below);
  }

  // Final column of first row, wherein we cannot fetch next values.
  ADV(current, 0);
  ADV(below, 0);
  next_map[width - 1] = col_step(above, current, below);

  // Remaining rows except the last.
  for (unsigned y = 1; y < height - 1; ++y) {
    unsigned offset = y * width;

    // Bootstrap row like we did for row 1.
    above[0] = above[1] = 0;
    current[0] = current[1] = 0;
    below[0] = below[1] = 0;

    above[2] = current_map[offset - width];
    current[2] = current_map[offset];
    below[2] = current_map[offset + width];

    for (unsigned x = 0; x < width - 1; ++x) {
      ADV(above, current_map[offset - width + x + 1]);
      ADV(current, current_map[offset + x + 1]);
      ADV(below, current_map[offset + width + x + 1]);
      next_map[offset + x] = col_step(above, current, below);
    }

    // Last column.
    ADV(above, 0);
    ADV(current, 0);
    ADV(below, 0);
    next_map[offset + width - 1] = col_step(above, current, below);
  }

  // Final row, wherein below[x] = 0.
  unsigned offset = width * (height - 1);
  above[0] = above[1] = 0;
  current[0] = current[1] = 0;
  below[0] = below[1] = below[2] = 0;

  above[2] = current_map[offset - width];
  current[2] = current_map[offset];

  for (unsigned x = 0; x < width - 1; ++x) {
    ADV(above, current_map[offset - width + x + 1]);
    ADV(current, current_map[offset + x + 1]);
    next_map[offset + x] = col_step(above, current, below);
  }

  // Final column
  ADV(above, 0);
  ADV(current, 0);
  next_map[offset + width - 1] = col_step(above, current, below);
}

static constexpr unsigned conway_cols = 800;
static constexpr unsigned conway_rows = 600;

static vga::rast::Bitmap_1 rasterizer(conway_cols, conway_rows);

// Cells will be set at boot when rand() is less than this.
static unsigned constexpr boot_threshold = 0x20000000;

// Seed and state variable for random number generation.
static unsigned seed = 1118;

// A simple linear congruential random number generator.
// (Coefficients borrowed from GCC.)
static unsigned rand() {
  seed = ((seed * 1103515245) + 12345) & 0x7FFFFFFF;
  return seed;
}

static void set_random_cells(unsigned threshold) {
  vga::Graphics1 g = rasterizer.make_bg_graphics();
  for (unsigned y = 0; y < conway_rows; ++y) {
    for (unsigned x = 0; x < conway_cols; ++x) {
      if (rand() < threshold) g.set_pixel(x, y);
    }
  }
}

static vga::Band const band = { &rasterizer, 600, nullptr };

void run(bool clear_first) {
  vga::sync_to_vblank();
  rasterizer.activate(vga::timing_vesa_800x600_60hz);
  rasterizer.set_fg_color(0b111111);
  rasterizer.set_bg_color(0b010000);

  vga::configure_band_list(&band);

  if (clear_first) {
    vga::Graphics1 g = rasterizer.make_bg_graphics();
    g.clear_all();
    set_random_cells(boot_threshold);
    rasterizer.flip();
  }

  input_init();
  vga::msigs_init();
  vga::video_on();

  while (!user_button_pressed()) {
    vga::msig_a_set();

    step(static_cast<unsigned const *>(rasterizer.get_fg_buffer()),
         static_cast<unsigned *>(rasterizer.get_bg_buffer()),
         conway_cols / 32,
         conway_rows);
  
    vga::msig_a_clear();

    if (rasterizer.can_bg_use_bitband()) {
      if (center_button_pressed()) set_random_cells(0x10000000);
    }

    vga::wait_for_vblank();
    rasterizer.flip_now();
  }

  vga::sync_to_vblank();
  vga::video_off();

  vga::configure_band_list(nullptr);
  vga::arena_reset();
}

}  // namespace conway
}  // namespace demo

