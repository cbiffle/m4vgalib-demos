#include "demo/conway/conway.h"

#include <cstdint>
#include <climits>

#include "etl/attribute_macros.h"
#include "etl/scope_guard.h"

#include "vga/rast/bitmap_1.h"
#include "vga/arena.h"
#include "vga/graphics_1.h"
#include "vga/measurement.h"
#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/input.h"

namespace demo {
namespace conway {

typedef std::uint32_t Unit;
static constexpr unsigned bits = 32;
static_assert(sizeof(Unit) * CHAR_BIT == bits,
              "Unit and bits must be adjusted together.");

/*
 * Result of a bit-parallel addition operation: a pair of bit vectors
 * representing sum and carry.
 */
struct AddResult {
  Unit sum;
  Unit carry;
};

/*
 * Bit-parallel half adder: adds corresponding bits of two 32-bit vectors,
 * producing sum and carry vectors.
 */
static constexpr ETL_INLINE AddResult half_add(Unit a, Unit b) {
  return { a ^ b, a & b };
}

/*
 * Bit-parallel full adder: add corresponding bits of *three* 32-bit vectors,
 * producing sum and carry vectors.
 */
static ETL_INLINE AddResult full_add(Unit a, Unit b, Unit c) {
  AddResult r0 = half_add(a, b);
  AddResult r1 = half_add(r0.sum, c);
  return { r1.sum, r0.carry | r1.carry };
}

/*
 * Step the automaton for the 32 cells contained in current[1], using the
 * neighboring bit vectors for context.
 */
ETL_SECTION(".ramcode")
ETL_INLINE
static Unit col_step(Unit above[3],
                     Unit current[3],
                     Unit below[3]) {
  /*
   * Compute row-wise influence sums.  This produces 96 2-bit sums (represented
   * as three pairs of 32-vectors) giving the number of live cells in the 1D
   * Moore neighborhood around each position.
   */
  AddResult a_inf = full_add((above[1] << 1) | (above[0] >> (bits - 1)),
                             above[1],
                             (above[1] >> 1) | (above[2] << (bits - 1)));
  AddResult c_inf = half_add((current[1] << 1) | (current[0] >> (bits - 1)),
                             /* middle bits of current[1] don't count */
                             (current[1] >> 1) | (current[2] << (bits - 1)));
  AddResult b_inf = full_add((below[1] << 1) | (below[0] >> (bits - 1)),
                             below[1],
                             (below[1] >> 1) | (below[2] << (bits - 1)));

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
ETL_SECTION(".ramcode")
static void step(Unit const *current_map,
                 Unit *next_map,
                 Unit width,
                 Unit height) {
  // We keep sliding windows of state in these arrays.
  Unit above[3] { 0, 0, 0 };
  Unit current[3] { 0, 0, 0 };
  Unit below[3] { 0, 0, 0 };

  // Bootstrap for first column of first row.
  current[0] = current[1] = 0;
  current[2] = current_map[0];

  below[0] = below[1] = 0;
  below[2] = current_map[width];

  #define ADV(name, next) \
    name[0] = name[1]; \
    name[1] = name[2]; \
    name[2] = (next)

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

  #undef ADV
}

struct Conway {
  static constexpr unsigned
    cols = 800,
    rows = 600;

  vga::rast::Bitmap_1 rasterizer{cols, rows};

  vga::Band const band { &rasterizer, rows, nullptr };

  // Seed and state variable for random number generation.
  unsigned seed = 1118;

  Conway() {
    rasterizer.set_fg_color(0b111111);
    rasterizer.set_bg_color(0b010000);
  }

  // A simple linear congruential random number generator.
  // (Coefficients borrowed from GCC.)
  unsigned rand() {
    seed = ((seed * 1103515245) + 12345) & 0x7FFFFFFF;
    return seed;
  }

  void set_random_cells(unsigned threshold) {
    vga::Graphics1 g = rasterizer.make_bg_graphics();
    for (unsigned y = 0; y < rows; ++y) {
      for (unsigned x = 0; x < cols; ++x) {
        if (rand() < threshold) g.set_pixel(x, y);
      }
    }
  }
};

// Cells will be set at boot when rand() is less than this.
static unsigned constexpr boot_threshold = 0x20000000;

void run(bool clear_first) {
  vga::arena_reset();
  input_init();
  vga::msigs_init();

  auto d = vga::arena_make<Conway>();

  auto & rasterizer = d->rasterizer;

  if (clear_first) {
    vga::Graphics1 g = rasterizer.make_bg_graphics();
    g.clear_all();
    d->set_random_cells(boot_threshold);
    rasterizer.flip_now();
  }

  vga::sync_to_vblank();
  vga::configure_band_list(&d->band);
  ETL_ON_SCOPE_EXIT { vga::clear_band_list(); };
  vga::video_on();

  while (!user_button_pressed()) {
    vga::msig_a_set();

    step(static_cast<Unit const *>(rasterizer.get_fg_buffer()),
         static_cast<Unit *>(rasterizer.get_bg_buffer()),
         Conway::cols / bits,
         Conway::rows);

    vga::msig_a_clear();

    if (rasterizer.can_bg_use_bitband()) {
      if (center_button_pressed()) d->set_random_cells(0x10000000);
    } else {
      // Luck of the arena; we can't provide this feature, it seems.
    }

    vga::wait_for_vblank();
    rasterizer.flip_now();
  }

  vga::sync_to_vblank();
  vga::video_off();
}

}  // namespace conway
}  // namespace demo

