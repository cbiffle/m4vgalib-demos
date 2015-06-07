#include "demo/tunnel/table.h"

#include <cmath>

#include "etl/integer_sequence.h"

namespace demo {
namespace tunnel {
namespace table {

/*
 * Welcome to some of the stranger C++ code I've ever written.
 *
 * We're writing in the constexpr dialect of C++11 here.  This means no locals,
 * no mutation, no loops.
 *
 * I'll annotate the idioms in use as we go along, but overall, we're going to
 * use the "compile-time index sequence" idiom.
 *
 * The idea: since we can't loop N times, we instead use varargs template
 * parameters to capture all the integers between 0 and N-1, and use pattern
 * expansion to calculate an entry for each integer.
 *
 * Note that the upcoming "generalized constexpr" support in a future version
 * of C++ should reduce the need for these gymnastics.
 */

/*
 * Calculates the table entry for the macroblock at pixel coordinate (x, y).
 *
 * TODO: this is imprecisely defined -- it's the corners, but we're kind of
 * making it sound like it's the whole block or the center.  Figure out how to
 * talk about this stuff.
 */
static constexpr PackedEntry make_element(float x, float y) {
  return PackedEntry { Entry {
    .distance = config::texture_period_d / sqrtf(x * x + y * y),
    .angle = config::texture_period_a * 0.5f
           * (std::atan2(y, x) / config::pi + 1),
  }};
}

/*
 * Facade for the float version that shifts coordinates to sample in the center
 * of the requested pixel.
 *
 * Note that this saves us from either repeating ourselves or needing locals
 * in the float version above.
 */
static constexpr PackedEntry make_element(unsigned x, unsigned y) {
  return make_element(x + 0.5f, y + 0.5f);
}

/*
 * Deconstruct an index sequence of X coordinates, and apply make_element to
 * each (X, Y) pair (where Y is fixed).
 *
 * Returns the result as a std::array.
 *
 * Note that this function can't be folded into generate_row, since it needs
 * an opportunity to capture the index sequence.
 */
template <std::size_t ... Is>
static constexpr Table::Row row_helper(unsigned y,
                                       etl::IndexSequence<Is...>) {
  return {{ make_element(config::sub * Is, config::sub * y)... }};
}

/*
 * Generates a single row (y) of the lookup table by constructing an index
 * sequence and processing it with row_helper.
 */
static constexpr Table::Row generate_row(unsigned y) {
  return row_helper(y, etl::MakeIndexSequence<width>{});
}

/*
 * Deconstruct an index sequence of Y coordinates and apply generate_row to
 * each.  Returns the result as a std::array.
 *
 * This is analogous to row_helper, above, and cannot be folded for the same
 * reason.
 */
template <std::size_t ... Is>
static constexpr Table::Array generate_helper(etl::IndexSequence<Is...>) {
  return {{ generate_row(Is)... }};
}

/*
 * Finally: produce and return our actual lookup table.  If called at runtime,
 * this would likely overflow the stack.  Fortunately, it's static, so all the
 * call sites are right here in this file -- and the only one is a constexpr
 * context, below.
 */
static constexpr Table::Array generate() {
  return generate_helper(etl::MakeIndexSequence<height>{});
}

/*
 * Create our Table by copying the constexpr Array literal.  By constructing it
 * as a constexpr value we have instructed the compiler to evaluate it.
 */
alignas(16) static Table constexpr the_table{generate()};

/*
 * Finally, the accessor.  By taking and returning the address of the constexpr
 * table value, we ensure that it is given a place in the application's address
 * space.  Our linker script will ensure that it goes into ROM.
 */
Table const & Table::compile_time_table() {
  return the_table;
}

}  // namespace table
}  // namespace tunnel
}  // namespace demo
