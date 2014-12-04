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
 * consists of one entry for each pixel in one quadrant of the screen -- we
 * derive the other quadrants from the first on the fly.
 * 
 * The lookup table and associated types are defined such that we can choose to
 * generate the lookup table either at demo launch, or at compile time,
 * depending on whether we have RAM or Flash to burn.
 */

static constexpr unsigned
  width = config::cols / 2,
  height = config::rows / 2;


/*
 * Each entry contains the cached results of two operations:
 *  - distance: the distance of the piece of the tunnel displayed on this pixel
 *    from the near clip plane (essentially a fixed Z-buffer).
 *  - angle: the angle from the X axis to the line between the center of the
 *    screen and this pixel.
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
 * (Single-precision floating point would be wasteful of space and cost very
 * little less than half-precision on the Cortex-M4.  Fixed point would be
 * equally compact, but actually more expensive to evaluate since we have an
 * FPU, and it also poses a greater analytical burden on the programmer.)
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
   * */
  explicit constexpr PackedEntry(uint32_t b) : packed_bits(b) {}

  /*
   * Packing constructor.
   */
  explicit constexpr PackedEntry(Entry const & e)
    : floats { __fp16(e.distance), __fp16(e.angle) } {}

  /*
   * Unpacking is not constexpr because (1) we don't need it at compile time and
   * (2) it's terribly performance sensitive.  So, it's in assembly.
   */
  Entry unpack() const {
    /*
     * GCC's half-precision floating point support (as of 4.8.3) only
     * generates vcvtb, so it misses an opportunity to efficiently unpack a
     * pair of half floats like this.  Because it's so speed critical we
     * resort to inline assembly:
     */
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
  using Array = std::array<PackedEntry, width * height>;

  Table() : _entries(generate_online()) {}

  constexpr Table(Array const & tmpl) : _entries(tmpl) {}

  static Table const compile_time_table;

  /*
   * Returns the unpacked Entry for a pixel location in Quadrant I.
   */
  Entry get(unsigned x, unsigned y) const {
    return _entries[y * width + x].unpack();
  }

private:
  std::array<PackedEntry, width * height> _entries;

  /*
   * Generates the lookup table contents for online mode.
   */
  static std::array<PackedEntry, width * height> generate_online();

};

}  // namespace table
}  // namespace tunnel
}  // namespace demo

#endif  // DEMO_TUNNEL_TABLE_H

