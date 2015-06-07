#include "demo/tunnel/table.h"

#include <cmath>

#include "etl/integer_sequence.h"

namespace demo {
namespace tunnel {
namespace table {

static constexpr PackedEntry make_element(float x, float y) {
  return PackedEntry { Entry {
    .distance = config::texture_period_d / sqrtf(x * x + y * y),
    .angle = config::texture_period_a * 0.5f
           * (std::atan2(y, x) / config::pi + 1),
  }};
}

static constexpr PackedEntry make_element(unsigned x, unsigned y) {
  return make_element(x + 0.5f, y + 0.5f);
}

/*
 * This function exists solely to deconstruct the index sequence produced in
 * generate, below.
 */
template <std::size_t ... Is>
static constexpr Table::Row row_helper(unsigned y,
                                       etl::IndexSequence<Is...>) {
  return {{ make_element(config::sub * Is, config::sub * y)... }};
}

/*
 * Generates one row of the lookup table as a literal array.
 */
static constexpr Table::Row generate_row(unsigned y) {
  return row_helper(y, etl::MakeIndexSequence<width>{});
}

template <std::size_t ... Is>
static constexpr Table::Array generate_helper(etl::IndexSequence<Is...>) {
  return {{ generate_row(Is)... }};
}

static constexpr Table::Array generate() {
  return generate_helper(etl::MakeIndexSequence<height>{});
}

alignas(16) Table const Table::compile_time_table{generate()};

}  // namespace table
}  // namespace tunnel
}  // namespace demo
