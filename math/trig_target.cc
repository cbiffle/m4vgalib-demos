#include "math/trig.h"

#include <array>
#include <cmath>

#include "etl/assert.h"
#include "etl/integer_sequence.h"

namespace math {

/*
 * Implementation factor of sin and cos.  Potentially generic.
 *
 * - 'angle' gives an angle in radians.  Its domain is not bounded, but values
 *   between 0 and 2pi will get best precision.
 * - 'negate' gives the factor to apply when the angle is negative -- -1 for
 *   sine, 1 for cosine.
 * - 'points' is the number of points in the lookup table.
 * - 'table' is the base address of the lookup table.
 *
 * Note: you might be inclined to change the namespaces below to structs and
 * templatize this operation on the struct type.  I certainly was.  But GCC
 * 4.8.3 (at least) has a hard time with the constexpr table generation if it
 * isn't at namespace scope.  So it goes.
 */
static inline float lookup_and_interpolate(float angle,
                                           float negate,
                                           unsigned points,
                                           __fp16 const * table) {
  if (angle < 0) {
    return negate * lookup_and_interpolate(-angle,
                                           negate,
                                           points,
                                           table);
  }

  auto domain = (angle / (2 * float(M_PI))) * points;
  float index_f;
  float frac = std::modf(domain, &index_f);

  auto index0 = (unsigned(index_f)) % points;
  auto index1 = (index0 + 1) % points;

  auto base = table[index0];
  auto slope = table[index1] - base;

  return base + slope * frac;
}


/*
 * Sine.
 */

namespace _sin {
  static constexpr unsigned points = 512;

  static constexpr float negate = -1;

  template <std::size_t ... is>
  static constexpr std::array<__fp16, sizeof...(is)>
      gen_(etl::IndexSequence<is...>) {
    return {{ std::sin(float(is) * (2 * float(M_PI)) / points)... }};
  }

  static constexpr std::array<__fp16, points> gen() {
    return gen_(etl::MakeIndexSequence<points>{});
  }

  __attribute__((section(".ramcode.math.sin_table")))
  static constexpr std::array<__fp16, points> table = gen();
};

float sin(float angle) {
  return lookup_and_interpolate(angle,
                                _sin::negate,
                                _sin::points,
                                &_sin::table[0]);
}


/*
 * Cosine.
 */

namespace _cos {
  static constexpr unsigned points = 512;

  static constexpr float negate = 1;

  template <std::size_t ... is>
  static constexpr std::array<__fp16, sizeof...(is)>
      gen_(etl::IndexSequence<is...>) {
    return {{ std::cos(float(is) * (2 * float(M_PI)) / points)... }};
  }

  static constexpr std::array<__fp16, points> gen() {
    return gen_(etl::MakeIndexSequence<points>{});
  }

  __attribute__((section(".ramcode.math.cos_table")))
  static constexpr std::array<__fp16, points> table = gen();
};

float cos(float angle) {
  return lookup_and_interpolate(angle,
                                _cos::negate,
                                _cos::points,
                                &_cos::table[0]);
}

}  // namespace math
