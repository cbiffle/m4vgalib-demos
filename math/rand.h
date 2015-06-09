#ifndef MATH_RAND_H
#define MATH_RAND_H

#include <cstdint>

namespace math {

// Generic function for getting a random element of type T.
template <typename T>
T rand();

// Produces a random number evenly distributed between 0..255.
template <> std::uint8_t rand<std::uint8_t>();
// Produces a random number evenly distributed between 0..65535.
template <> std::uint16_t rand<std::uint16_t>();
// Produces a random number evenly distributed between 0..1.
template <> float rand<float>();

}  // namespace math

#endif  // MATH_RAND_H
