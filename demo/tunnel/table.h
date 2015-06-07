#ifndef DEMO_TUNNEL_TABLE_H
#define DEMO_TUNNEL_TABLE_H

#include <array>
#include <cstdint>

#include "demo/tunnel/config.h"

namespace demo {
namespace tunnel {
namespace table {

/*
 * This lookup table eliminates transcendentals from the render loop.  It
 * consists of one entry for each "macroblock" in one quadrant of the screen --
 * we derive the other quadrants from the first on the fly.
 *
 * "Macroblocks" here consist of square areas, config::sub pixels on a side.
 * We bilinearly interpolate between table samples taken at macroblock corners.
 * This reduces the size of the lookup table by a factor of config::sub^2.
 * Of course, it's also wrong -- in the sense that neither function we're
 * evaluating is linear.  However, at relatively small values of config::sub,
 * it's very difficult to see the errors.
 * 
 * The lookup table and associated types are defined such that we can produce
 * them entirely at compile time using constexpr, ensuring that they can go
 * into Flash.  The algorithms used at runtime are designed to avoid the
 * resulting wait states.
 */

static constexpr unsigned
  width = config::quad_width / config::sub + 1,
  height = config::quad_height / config::sub + 1;


/*
 * Each entry contains the cached results of two operations:
 *  - distance: the distance from the near clip plane to the piece of tunnel
 *    displayed in this block -- essentially a precomputed Z-buffer.
 *  - angle: the angle from the X axis to the line between the center of the
 *    screen and this block.
 *
 * Neither of these are measured in real-world units -- instead, they're derived
 * from the texturing parameters to ease their (ab)use as texture (U, V)
 * coordinates.
 */
struct Entry {
  float distance;
  float angle;
};

/*
 * We store the pairs in a compact format using half-precision floating point.
 * The Cortex-M4 has hardware support for packing and unpacking pairs of
 * half-precision floating point values, so this costs basically nothing.
 *
 * Single-precision floating point takes twice as much space without visibly
 * improving the output.
 *
 * The demo was originally fixed point, but it was hard on both the programmer
 * and the CPU: the programmer had to track the range and scale of each
 * intermediate value, and the CPU couldn't use its *really fast* FPU.  So
 * believe it or not, this is significantly faster than fixed point.
 *
 * Because GCC 4.8.3 has pretty limited ability to pack half-precision floats
 * into words when moving data to/from memory, we do it manually by representing
 * this as a union.
 */
union PackedEntry {
  std::uint32_t packed_bits;
  struct {
    __fp16 distance;
    __fp16 angle;
  } floats;

  /*
   * Default constructor leaves things uninitialized; this is here purely
   * because it's useful in the implementation.
   * */
  PackedEntry() {}

  /*
   * Bitwise conversion constructor.
   */
  explicit constexpr PackedEntry(uint32_t b) : packed_bits(b) {}

  /*
   * Packing constructor.
   */
  explicit constexpr PackedEntry(Entry const & e)
    : floats { __fp16(e.distance), __fp16(e.angle) } {}

  /*
   * GCC's half-precision floating point support (as of 4.8.3, anyway) will only
   * produce code for unpacking from the bottom half of a word -- but the M4F
   * can efficiently unpack both halves in one more cycle.  So we resort to
   * assembly here.
   *
   * This means unpacking can't be constexpr, but we don't actually need it at
   * compile time -- so that's fine.
   */
  Entry unpack() const {
    float distance, angle;
    asm (
      "vcvtt.f32.f16 %[angle], %[bits]\n"
      "vcvtb.f32.f16 %[distance], %[bits]\n"
    : [distance] "=w"  (distance),
      [angle]    "=&w" (angle)
    : [bits]     "w"   (packed_bits)
    );
    return { distance, angle };
  }
};

/*
 * Aaaand the actual table implementation.
 */
class Table {
public:
  // Convenient shorthand for our fixed-size arrays.
  using Row = std::array<PackedEntry, width>;
  using Array = std::array<Row, height>;

  /*
   * Creates a Table by *copying* an Array.  This is not something you want to
   * do accidentally at runtime.  Fortunately, it's pretty hard for you to
   * acquire a value of type Array without deliberately pointing a gun at your
   * foot.
   */
  constexpr Table(Array const & tmpl) : _entries(tmpl) {}

  /*
   * You really don't want to copy a Table.
   */
  Table(Table const &) = delete;

  /*
   * Accessor for the ROM table.
   */
  static Table const & compile_time_table();

  /*
   * Returns the unpacked Entry for a pixel location in Quadrant I.
   */
  Entry get(unsigned x, unsigned y) const {
    return _entries[y][x].unpack();
  }

private:
  Array _entries;
};

}  // namespace table
}  // namespace tunnel
}  // namespace demo

#endif  // DEMO_TUNNEL_TABLE_H

