#include "demo/tunnel/table.h"

#include <cmath>

#include "etl/integer_sequence.h"

namespace demo {
namespace tunnel {
namespace table {

static constexpr PackedEntry make_element(float x, float y) {
  return PackedEntry { Entry {
    .distance = config::texture_period_d / std::sqrt(x * x + y * y),
    .angle = config::texture_period_a * 0.5f
           * (std::atan2(y, x) / config::pi + 1),
  }};
}

static constexpr PackedEntry make_element(std::size_t index) {
  return make_element(index % width + 0.5f, index / width + 0.5f);
}

auto Table::generate_online() -> std::array<PackedEntry, width * height> {
  std::array<PackedEntry, width * height> table;

  for (unsigned i = 0; i < width * height; ++i) {
    table[i] = make_element(i);
  }

  return table;
}

/*
 * This function exists solely to deconstruct the index sequence produced in
 * generate, below.
 */
template <std::size_t ... Is>
static constexpr std::array<PackedEntry, sizeof...(Is)>
    generate_helper(etl::IndexSequence<Is...>) {
  return {{ make_element(Is)... }};
}

/*
 * Generates the lookup table contents for literal mode, as a compile-time
 * literal array.
 */
static constexpr std::array<PackedEntry, width * height> generate() {
  return generate_helper(etl::MakeIndexSequence<width * height>{});
}

Table const Table::compile_time_table{generate()};

}  // namespace table
}  // namespace tunnel
}  // namespace demo
