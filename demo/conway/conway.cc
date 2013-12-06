
#include "lib/common/attribute_macros.h"

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
static INLINE AddResult half_add(unsigned a, unsigned b) {
  return { a ^ b, a & b };
}

/*
 * Bit-parallel full adder: add corresponding bits of *three* 32-bit vectors,
 * producing sum and carry vectors.
 */
static INLINE AddResult full_add(unsigned a, unsigned b, unsigned c) {
  AddResult r0 = half_add(a, b);
  AddResult r1 = half_add(r0.sum, c);
  return { r1.sum, r0.carry | r1.carry };
}

/*
 * Step the automaton for the 32 cells contained in current[1], using the
 * neighboring bit vectors for context.
 */
__attribute__((section(".ramcode")))
static INLINE unsigned col_step(unsigned above[3],
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

void step(unsigned const *current_map,
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
