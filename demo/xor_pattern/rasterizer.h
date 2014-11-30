#ifndef DEMO_XOR_PATTERN_RASTERIZER_H
#define DEMO_XOR_PATTERN_RASTERIZER_H

#include "vga/rasterizer.h"

namespace demo {
namespace xor_pattern {

class Rasterizer : public vga::Rasterizer {
public:
  Rasterizer(vga::Timing const &);
  virtual LineShape rasterize(unsigned, Pixel *);

private:
  unsigned _width;
  unsigned _frame;
};

}  // namexpace xor_pattern
}  // namespace demo

#endif  // DEMO_XOR_PATTERN_RASTERIZER_H
