#include "lib/armv7m/exception_table.h"

#include "runtime/startup.h"

#include "vga/graphics_1.h"
#include "vga/timing.h"
#include "vga/vga.h"
#include "vga/rast/bitmap_1.h"
#include "vga/rast/text_10x16.h"

using vga::rast::Bitmap_1;

typedef vga::Rasterizer::Pixel Pixel;


/*******************************************************************************
 * Screen partitioning
 */

static constexpr unsigned text_cols = 800;
static constexpr unsigned text_rows = 16 * 16;

static constexpr unsigned gfx_cols = 800;
static constexpr unsigned gfx_rows = 600 - text_rows;


/*******************************************************************************
 * The Graphics Parts
 */

static Bitmap_1 gfx_rast(gfx_cols, gfx_rows);

class Particle {
public:
  static constexpr unsigned lifetime = 120;
  static unsigned seed;

  void randomize() {
    _x[0] = _x[1] = rand() % gfx_cols;
    _y[0] = _y[1] = rand() % (gfx_rows * 2/3);
    _dx = rand() % 9 - 5;
    _dy = rand() % 3 - 2;
  }

  void nudge(int ddx, int ddy) {
    _dx += ddx;
    _dy += ddy;
  }

  void step(vga::Graphics1 &g) {
    g.clear_pixel(_x[1], _y[1]);
    g.clear_pixel(_x[1] - 1, _y[1]);
    g.clear_pixel(_x[1] + 1, _y[1]);
    g.clear_pixel(_x[1], _y[1] - 1);
    g.clear_pixel(_x[1], _y[1] + 1);

    _x[1] = _x[0];
    _y[1] = _y[0];

    int x_ = _x[0] + _dx;
    int y_ = _y[0] + _dy;

    if (x_ < 0) {
      x_ = 0;
      _dx = -_dx;
    }

    if (y_ < 0) {
      y_ = 0;
      _dy = -_dy;
    }

    if (x_ >= static_cast<int>(gfx_cols)) {
      x_ = gfx_cols - 1;
      _dx = -_dx;
    }

    if (y_ >= static_cast<int>(gfx_rows)) {
      y_ = gfx_rows - 1;
      _dy = -_dy;
    }

    _x[0] = x_;
    _y[0] = y_;

    g.set_pixel(_x[0], _y[0]);
    g.set_pixel(_x[0] - 1, _y[0]);
    g.set_pixel(_x[0] + 1, _y[0]);
    g.set_pixel(_x[0], _y[0] - 1);
    g.set_pixel(_x[0], _y[0] + 1);
  }

private:
  int _x[2], _y[2];
  int _dx, _dy;

  unsigned rand() {
    seed = (seed * 1664525) + 1013904223;
    return seed;
  }
};

unsigned Particle::seed = 1118;

static constexpr unsigned particle_count = 500;

static Particle particles[particle_count];

static void update_particles() {
  vga::Graphics1 g = gfx_rast.make_bg_graphics();
  for (Particle &p : particles) {
    p.step(g);
    p.nudge(0, 1);
  }
  gfx_rast.flip();
}


/*******************************************************************************
 * The Text Parts
 */

static vga::rast::Text_10x16 text_rast(text_cols, text_rows, gfx_rows);


static constexpr unsigned t_right_margin = (text_cols + 9) / 10;
static constexpr unsigned t_bottom_margin = (text_rows + 15) / 16;

static unsigned t_row = 0, t_col = 0;

static void type_raw(Pixel fore, Pixel back, char c) {
  text_rast.put_char(t_col, t_row, fore, back, c);
  ++t_col;
  if (t_col == t_right_margin) {
    t_col = 0;
    ++t_row;
    if (t_row == t_bottom_margin) t_row = 0;
  }
}

static void type(Pixel fore, Pixel back, char c) {
  switch (c) {
    case '\n':
      do {
        type_raw(fore, back, ' ');
      } while (t_col);
      return;

    default:
      type_raw(fore, back, c);
      return;
  }
}

static void type(Pixel fore, Pixel back, char const *s) {
  while (char c = *s++) {
    type(fore, back, c);
  }
}

static void rainbow_type(char const *s) {
  unsigned x = 0;
  while (char c = *s++) {
    type(x & 0b111111, ~x & 0b111111, c);
    ++x;
  }
}

static void cursor_to(unsigned col, unsigned row) {
  if (col >= t_right_margin) col = t_right_margin - 1;
  if (row >= t_bottom_margin) row = t_bottom_margin - 1;

  t_col = col;
  t_row = row;
}

static void text_at(unsigned col, unsigned row,
                    Pixel fore, Pixel back,
                    char const *s) {
  cursor_to(col, row);
  type(fore, back, s);
}

static void text_centered(unsigned row, Pixel fore, Pixel back, char const *s) {
  unsigned len = 0;
  while (s[len]) ++len;

  unsigned left_margin = 40 - len / 2;
  unsigned right_margin = t_right_margin - len - left_margin;

  cursor_to(0, row);
  for (unsigned i = 0; i < left_margin; ++i) type(fore, back, ' ');
  type(fore, back, s);
  for (unsigned i = 0; i < right_margin; ++i) type(fore, back, ' ');
}

enum {
  white   = 0b111111,
  lt_gray = 0b101010,
  dk_gray = 0b010101,
  black   = 0b000000,

  red     = 0b000011,
  green   = 0b001100,
  blue    = 0b110000,
};


/*******************************************************************************
 * The Startup Routine
 */

void v7m_reset_handler() {
  crt_init();
  vga::init();

  gfx_rast.activate(vga::timing_vesa_800x600_60hz);
  gfx_rast.set_fg_color(0b111111);
  gfx_rast.set_bg_color(0b100000);

  text_rast.activate(vga::timing_vesa_800x600_60hz);
  text_rast.clear_framebuffer(0);

  text_centered(0, white, dk_gray, "800x600 Mixed Graphics Modes Demo");
  cursor_to(0, 2);
  type(white, black, "Bitmap framebuffer combined with ");
  type(black, white, "attributed");
  type(red, black, " 256");
  type(green, black, " color ");
  type(blue, black, "text.");
  cursor_to(0, 4);
  rainbow_type("The quick brown fox jumped over the lazy dog. "
               "0123456789!@#$%^{}");

  text_at(0, 6, white, 0b100000,
      "     Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam ut\n"
      "     tellus quam. Cras ornare facilisis sollicitudin. Quisque quis\n"
      "     imperdiet mauris. Proin malesuada nibh dolor, eu luctus mauris\n"
      "     ultricies vitae. Interdum et malesuada fames ac ante ipsum primis\n"
      "     in faucibus. Aenean tincidunt viverra ultricies. Quisque rutrum\n"
      "     vehicula pulvinar.\n");

  text_at(0, 15, white, black, "60 fps / 40MHz pixel clock");
  text_at(58, 36, white, black, "Frame number:");

  vga::configure_band(0, gfx_rows, &gfx_rast);
  vga::configure_band(gfx_rows, text_rows, &text_rast);
  vga::configure_timing(vga::timing_vesa_800x600_60hz);

  for (Particle &p : particles) p.randomize();

  char fc[9];
  fc[8] = 0;
  unsigned frame = 0;

  while (1) {
    unsigned f = ++frame;

    for (unsigned i = 8; i > 0; --i) {
      unsigned n = f & 0xF;
      fc[i - 1] = n > 9 ? 'A' + n - 10 : '0' + n;
      f >>= 4;
    }
    text_at(72, 15, red, black, fc);
    update_particles();

    f = ++frame;
    for (unsigned i = 8; i > 0; --i) {
      unsigned n = f & 0xF;
      fc[i - 1] = n > 9 ? 'A' + n - 10 : '0' + n;
      f >>= 4;
    }
    text_at(72, 15, red, black, fc);
    update_particles();
  }
}
