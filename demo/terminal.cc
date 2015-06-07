#include "demo/terminal.h"

#include "vga/font_10x16.h"

namespace demo {

Terminal::Terminal(unsigned width, unsigned height, unsigned top_line)
  : rasterizer(vga::font_10x16, 256, width, height, top_line),
    t_row(0), t_col(0) {
  rasterizer.clear_framebuffer(0);
}

void Terminal::type_raw(Pixel fore, Pixel back, char c) {
  rasterizer.put_char(t_col, t_row, fore, back, c);
  ++t_col;
  if (t_col == rasterizer.get_col_count()) {
    t_col = 0;
    ++t_row;
    if (t_row == rasterizer.get_row_count()) t_row = 0;
  }
}

void Terminal::type(Pixel fore, Pixel back, char c) {
  switch (c) {
    case '\r':
      do {
        type_raw(fore, back, ' ');
      } while (t_col);
      return;

    case '\n':
      do {
        type_raw(fore, back, ' ');
      } while (t_col);
      return;

    case '\f':
      rasterizer.clear_framebuffer(back);
      cursor_to(0, 0);
      return;

    case '\b':
      if (t_col) {
        --t_col;
        type_raw(fore, back, ' ');
        --t_col;
      }
      return;

    default:
      type_raw(fore, back, c);
      return;
  }
}

void Terminal::type(Pixel fore, Pixel back, char const *s) {
  while (char c = *s++) {
    type(fore, back, c);
  }
}

void Terminal::cursor_to(unsigned col, unsigned row) {
  if (col >= rasterizer.get_col_count()) col = rasterizer.get_col_count() - 1;
  if (row >= rasterizer.get_row_count()) row = rasterizer.get_row_count() - 1;

  t_col = col;
  t_row = row;
}

void Terminal::text_at(unsigned col, unsigned row,
                       Pixel fore, Pixel back,
                       char const *s) {
  cursor_to(col, row);
  type(fore, back, s);
}

void Terminal::text_centered(unsigned row,
                             Pixel fore,
                             Pixel back,
                             char const *s) {
  unsigned len = 0;
  while (s[len]) ++len;

  unsigned left_margin = rasterizer.get_col_count() / 2 - len / 2;
  unsigned right_margin = rasterizer.get_col_count() - len - left_margin;

  cursor_to(0, row);
  for (unsigned i = 0; i < left_margin; ++i) type(fore, back, ' ');
  type(fore, back, s);
  for (unsigned i = 0; i < right_margin; ++i) type(fore, back, ' ');
}

void Terminal::rainbow_type(char const *s) {
  unsigned x = 0;
  while (char c = *s++) {
    type(x & 0b111111, ~x & 0b111111, c);
    ++x;
  }
}


}  // namespace demo
