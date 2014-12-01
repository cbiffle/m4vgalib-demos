#ifndef DEMO_TERMINAL_H
#define DEMO_TERMINAL_H

#include "vga/rast/text_10x16.h"
#include "vga/rasterizer.h"

namespace demo {

/*******************************************************************************
 * Some basic terminal functionality.
 */

enum {
  white   = 0b111111,
  lt_gray = 0b101010,
  dk_gray = 0b010101,
  black   = 0b000000,

  red     = 0b000011,
  green   = 0b001100,
  blue    = 0b110000,
};

struct Terminal {
  using Pixel = vga::Rasterizer::Pixel;

  vga::rast::Text_10x16 rasterizer;

  unsigned t_row, t_col;

  Terminal(unsigned width, unsigned height, unsigned top_line = 0);

  void type_raw(Pixel fore, Pixel back, char c);

  void type(Pixel fore, Pixel back, char c);

  void type(Pixel fore, Pixel back, char const *s);

  void cursor_to(unsigned col, unsigned row);

  void text_at(unsigned col, unsigned row,
               Pixel fore, Pixel back,
               char const *s);

  void text_centered(unsigned row, Pixel fore, Pixel back, char const *s);

  void rainbow_type(char const *);
};

}  // namespace demo

#endif // DEMO_TERMINAL_H
