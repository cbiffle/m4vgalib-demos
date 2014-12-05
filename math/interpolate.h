#ifndef MATH_INTERPOLATE_H
#define MATH_INTERPOLATE_H

namespace math {

/*
 * Given an amount between 0 and 1, interpolates between first and second.
 *
 * Amounts less than 0 or greater than 1 will continue along the line between
 * first and second; if you want edge-pinning or repetition, you must implement
 * it on top of this primitive.
 */
template <typename T>
T linear_interpolate(T first, T second, float amount) {
  T delta = second - first;
  return first + (delta * amount);
}

/*
 * Applies linear_interpolate along two axes, x and y.
 */
template <typename T>
T bilinear_interpolate(T bottom_left, T bottom_right,
                       T top_left, T top_right,
                       float x, float y) {
  T bottom = linear_interpolate(bottom_left, bottom_right, x);
  T top    = linear_interpolate(top_left,    top_right,    x);
  return linear_interpolate(bottom, top, y);
}

}  // namespace math

#endif  // MATH_INTERPOLATE_H
