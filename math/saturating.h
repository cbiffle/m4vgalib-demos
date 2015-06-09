#ifndef MATH_SATURATING_H
#define MATH_SATURATING_H

#include <cstdint>

#if defined(__ARM_ARCH)
#include "etl/armv7m/instructions.h"
#endif

namespace math {

template <unsigned N>
std::int32_t signed_saturate(std::int32_t in) {
#if defined(__ARM_FEATURE_SAT) && __ARM_FEATURE_SAT == 1
  return etl::armv7m::ssat<N>(in);
#else
  auto max = int32_t(1 << (N - 1));
  if (in > max) return max;
  if (in < (-1 - max)) return -1 - max;
  return in;
#endif
}

}  // namespace math

#endif  // MATH_SATURATING_H
