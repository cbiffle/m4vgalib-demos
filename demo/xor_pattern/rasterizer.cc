#include "demo/xor_pattern/rasterizer.h"

#include "etl/attribute_macros.h"

#include "demo/xor_pattern/pattern.h"

namespace demo {
namespace xor_pattern {

Rasterizer::Rasterizer(unsigned width)
  : _width(width),
    _frame(0) {}

ETL_SECTION(".ramcode")
auto Rasterizer::rasterize(unsigned line_number, Pixel *target) -> RasterInfo {
  unsigned f = _frame;

  if (line_number == 0) _frame = ++f;

  pattern((line_number >> 2) + f, f, target, _width);

  return {
    .offset = 0,
    .length = _width,
    .stretch_cycles = 0,
    .repeat_lines = 0,
  };
}

}  // namespace xor_pattern
}  // namespace demo
