#ifndef LIB_COMMON_ALGORITHM_H
#define LIB_COMMON_ALGORITHM_H

namespace common {

template <typename T>
T const min(T const &a, T const &b) {
  return a < b ? a : b;
}

template <typename T>
T const max(T const &a, T const &b) {
  return a > b ? a : b;
}

}  // namespace common

#endif  // LIB_COMMON_ALGORITHM_H
