#include "demo/rook/rook.h"

#include "etl/assert.h"
#include "etl/scope_guard.h"

#include "vga/arena.h"
#include "vga/graphics_1.h"
#include "vga/measurement.h"
#include "vga/rast/bitmap_1.h"
#include "vga/rast/solid_color.h"
#include "vga/rast/text_10x16.h"
#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/input.h"

#include "demo/rook/geometry.h"
#include "demo/rook/model.h"

#include <math.h>

namespace demo {
namespace rook {

using Pixel = vga::Rasterizer::Pixel;

/*
 * We exploit the existence of large blank regions at the top and bottom of
 * the screen to use trivial rasterizers and save CPU.
 *   Top 100: blue
 *   Middle 400: wireframe
 *   Next 83: blue
 *   Final 16: text
 */
static constexpr unsigned
  cols = 800,
  rows = 600,
  top_margin = 100,
  wireframe_rows = 400,
  bottom_margin = 83,
  text_rows = 17;



/*******************************************************************************
 * Line drawing using a vertex buffer.
 *
 * I started out using normal binary STL, but it has a few problems:
 *  1. The bastards designed the records so that half the vertices are
 *     unaligned.  This really hampers the options for code generation --
 *     almost any optimization causes a usage fault.
 *  2. As usual with STL, it's a bag-of-facets: there's no adjacency info.
 *     This leads to massive overdraw.
 *
 * Instead, I'm using something optimized for wireframe rendering.  I transform
 * all unique vertices (of which there are not that many, after processing) and
 * then draw edges using indices into the vertex table.  It's rather like
 * glDrawElements.
 */

struct Wireframe {
  vga::rast::Bitmap_1 rasterizer{cols,
                                 wireframe_rows,
                                 top_margin};
  Vec2i *transformed_vertices;

  Wireframe()
    : transformed_vertices(vga::arena_new_array<Vec2i>(vertex_count)) {
    rasterizer.set_fg_color(0b111111);
    rasterizer.set_bg_color(0b010000);

    // Switch to whichever page has bitband support.
    if (!rasterizer.can_bg_use_bitband()) {
      rasterizer.flip();
      ETL_ASSERT(rasterizer.can_bg_use_bitband());
    }
  }

  ~Wireframe() {
    transformed_vertices = nullptr;
  }

  __attribute__((section(".ramcode.transform_vertices")))
  void transform_vertices(Mat4f const &m) const {
    for (unsigned i = 0; i < vertex_count; ++i) {
      Vec3f v = (m * static_cast<Vec4f>(vertices[i])).project();
      transformed_vertices[i] = { static_cast<int>(v.x),
                                  static_cast<int>(v.y) };
    }
  }

  __attribute__((section(".ramcode.draw_edges")))
  void draw_edges(vga::Graphics1 &g) {
    for (unsigned i = 0; i < edge_count; ++i) {
      Vec2i const &a = transformed_vertices[edges[i][0]];
      Vec2i const &b = transformed_vertices[edges[i][1]];

      g.set_line_unclipped(a.x, a.y, b.x, b.y);
    }
  }
};

/*******************************************************************************
 * Text rendering for scrolling brag line
 */

static char const spaces[80] = { ' ' };

struct BragLine {
  unsigned message[81];
  unsigned t_c = 0;
  vga::rast::Text_10x16 text{cols - 10, text_rows, rows - text_rows};

  BragLine() {
    prepare_message();
  }

  void string(Pixel fore, Pixel back, char const *s) {
    while (char c = *s++) {
      if (c == 0) break;
      message[t_c++] = (fore << 16) | (back << 8) | c;
    }
  }

  void prepare_message() {
    t_c = 0;
    string(0b111111, 0b010000, "2450 triangles - ");
    string(0b111111, 0b110000, "5.9Mpix/sec perspective projected fill rate");
    string(0b111111, 0b010000, " - ");
    string(0b000011, 0b010000, "60fps @ 800x600");
    string(0b111111, 0b010000, " - ");
  }

  void show_msg(unsigned frame) {
    text.set_x_adj(9 - (frame % 10));

    frame /= 10;
    frame %= 81;
    text.clear_framebuffer(0b010000);
    for (unsigned i = 0; i < 81 - frame; ++i) {
      text.put_packed(i, 0, message[frame + i]);
    }
    for (unsigned i = 0; i < frame; ++i) {
      text.put_packed(81 - frame + i, 0, message[i]);
    }
  }
};

/*******************************************************************************
 * Combination.
 */

struct Demo {
  vga::rast::SolidColor blue{cols, 0b010000};
  Wireframe wireframe;
  BragLine brag_line;

  vga::Band const bands[6] {
    { &blue,                 1,                 &bands[1] },
    { nullptr,               top_margin - 1,    &bands[2] },
    { &wireframe.rasterizer, wireframe_rows,    &bands[3] },
    { &blue,                 1,                 &bands[4] },
    { nullptr,               bottom_margin - 1, &bands[5] },
    { &brag_line.text,       text_rows,         nullptr },
  };
};

/*******************************************************************************
 * The main bits.
 */


__attribute__((section(".ramcode.rook_run")))
void run() {
  vga::arena_reset();

  input_init();

  Mat4f m = Mat4f::identity();
  // Translate world-space to screen-space.
  m = m * Mat4f::translate(400, 200, 0);
  m = m * Mat4f::scale(300, 300, 1);
  // Project.
  m = m * Mat4f::persp(-10, -10, 10, 10, 20, 100);
  // Translate world away from camera.
  m = m * Mat4f::translate(0, 0, -70);

  Mat4f m2 = Mat4f::identity();

  auto d = vga::arena_make<Demo>();

  vga::configure_band_list(d->bands);
  ETL_ON_SCOPE_EXIT { vga::clear_band_list(); };

  vga::wait_for_vblank();
  vga::video_on();
  ETL_ON_SCOPE_EXIT {
    vga::wait_for_vblank();
    vga::video_off();
  };

  // Because we copy, not flip, this can be hoisted out of loop.
  vga::Graphics1 g = d->wireframe.rasterizer.make_bg_graphics();

  unsigned frame = 0;
  while (!user_button_pressed()) {
    ++frame;

    d->brag_line.show_msg(frame);

    auto j = read_joystick();

    if (j & JoyBits::up) {
      m2 = m2 * Mat4f::rotate_z(-0.01);
    } else if (j & JoyBits::down) {
      m2 = m2 * Mat4f::rotate_z(0.01);
    }

    if (j & JoyBits::left) {
      m = m * Mat4f::rotate_y(-0.01);
    } else if (j & JoyBits::right) {
      m = m * Mat4f::rotate_y(0.01);
    }

    g.clear_all();
    d->wireframe.transform_vertices(m * m2);
    vga::msig_a_set();
    d->wireframe.draw_edges(g);
    vga::msig_a_clear();

    vga::wait_for_vblank();
    d->wireframe.rasterizer.copy_bg_to_fg();
  }
}

}  // namespace rook
}  // namespace demo
