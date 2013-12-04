#include "demo/xor_pattern/xor.h"

#include "vga/timing.h"

#include "demo/xor_pattern/pattern.h"

namespace demo {
namespace xor_pattern {

void Xor::activate(vga::Timing const &timing) {
  _frame = 0;
  _width = timing.video_pixels;
}

__attribute__((section(".ramcode")))
vga::Rasterizer::LineShape Xor::rasterize(unsigned line_number, Pixel *target) {
  unsigned f = _frame;

  if (line_number == 0) _frame = ++f;

  pattern((line_number >> 2) + f, f, target, _width);

  return { 0, _width };
}

}  // namespace xor_pattern
}  // namespace demo
