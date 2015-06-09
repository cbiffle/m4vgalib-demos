#include "math/rand.h"

namespace math {

static std::uint32_t rand_seed = 1118;

static std::uint32_t advance_generator() {
  rand_seed = (rand_seed * 1103515245) + 12345;
  return rand_seed;
}

template<>
std::uint8_t rand<std::uint8_t>() {
  auto s = advance_generator();
  return (s >> 4) & 0xFF;
}

template<>
std::uint16_t rand<std::uint16_t>() {
  auto s = advance_generator();
  return (s >> 4) & 0xFFFF;
}

template<>
float rand<float>() {
  auto s = advance_generator();
  return float(s) / float(1ull << 32);
}

}  // namespace math
