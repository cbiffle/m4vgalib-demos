#ifndef DEMO_XOR_PATTERN_XOR_H
#define DEMO_XOR_PATTERN_XOR_H

#include "vga/rasterizer.h"

namespace demo {
namespace xor_pattern {

class Xor : public vga::Rasterizer {
public:
  virtual void activate(vga::Timing const &);
  virtual LineShape rasterize(unsigned, Pixel *);

private:
  unsigned _width;
  unsigned _frame;
};

}  // namexpace xor_pattern
}  // namespace demo

#endif  // DEMO_XOR_PATTERN_XOR_H
