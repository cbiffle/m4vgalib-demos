#include "demo/xor_pattern/rasterizer.h"

#include "demo/xor_pattern/pattern.h"

namespace demo {
namespace xor_pattern {

Rasterizer::Rasterizer(unsigned width)
  : _width(width),
    _frame(0) {}

__attribute__((section(".ramcode")))
vga::Rasterizer::LineShape Rasterizer::rasterize(unsigned line_number,
                                                 Pixel *target) {
  unsigned f = _frame;

  if (line_number == 0) _frame = ++f;

  pattern((line_number >> 2) + f, f, target, _width);

  return { 0, _width };
}

}  // namespace xor_pattern
}  // namespace demo
