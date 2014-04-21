#include "demo/rook/rook.h"

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

/*
 * We exploit the existence of large blank regions at the top and bottom of
 * the screen to use trivial rasterizers and save CPU.
 *   Top 100: blue
 *   Middle 400: wireframe
 *   Next 83: blue
 *   Final 16: text
 */
static vga::rast::SolidColor blue(800, 0b010000);
static vga::rast::Bitmap_1 rasterizer(800, 500 - 17, 100);
static vga::rast::Text_10x16 text(800 - 10, 17, 600 - 17);

static vga::Band const bands[] = {
  { &blue, 1, &bands[1] },
  { nullptr, 99, &bands[2] },
  { &rasterizer, 400, &bands[3] },
  { &blue, 1, &bands[4] },
  { nullptr, 99 - 17, &bands[5] },
  { &text, 17, nullptr },
};


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

static Vec2i *transformed_vertices;

static void allocate_vertex_table() {
  transformed_vertices = new Vec2i[vertex_count];
}

__attribute__((section(".ramcode.transform_vertices")))
static void transform_vertices(Mat4f const &m) {
  for (unsigned i = 0; i < vertex_count; ++i) {
    Vec3f v = (m * static_cast<Vec4f>(vertices[i])).project();
    transformed_vertices[i] = { static_cast<int>(v.x),
                                static_cast<int>(v.y) };
  }
}

__attribute__((section(".ramcode.draw_edges")))
static void draw_edges(vga::Graphics1 &g) {
  for (unsigned i = 0; i < edge_count; ++i) {
    Vec2i const &a = transformed_vertices[edges[i][0]];
    Vec2i const &b = transformed_vertices[edges[i][1]];

    g.set_line_unclipped(a.x, a.y, b.x, b.y);
  }
}


/*******************************************************************************
 * Text rendering for scrolling brag line
 */

static char const spaces[80] = { ' ' };

static unsigned message[81];

static unsigned t_c = 0;

typedef vga::Rasterizer::Pixel Pixel;

static void string(Pixel fore, Pixel back, char const *s) {
  while (char c = *s++) {
    if (c == 0) break;
    message[t_c++] = (fore << 16) | (back << 8) | c;
  }
}

static void prepare_message() {
  t_c = 0;
  string(0b111111, 0b010000, "2450 triangles - ");
  string(0b111111, 0b110000, "5.9Mpix/sec perspective projected fill rate");
  string(0b111111, 0b010000, " - ");
  string(0b000011, 0b010000, "60fps @ 800x600");
  string(0b111111, 0b010000, " - ");
}

static void show_msg(unsigned frame) {
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

/*******************************************************************************
 * The main bits.
 */


__attribute__((section(".ramcode.rook_run")))
void run() {
  rasterizer.activate(vga::timing_vesa_800x600_60hz);
  rasterizer.set_fg_color(0b111111);
  rasterizer.set_bg_color(0b010000);

  allocate_vertex_table();

  text.activate(vga::timing_vesa_800x600_60hz);
  prepare_message();

  vga::configure_band_list(&bands[0]);

  // Switch to whichever page has bitband support.
  if (!rasterizer.can_bg_use_bitband()) {
    rasterizer.flip();
    if (!rasterizer.can_bg_use_bitband()) while (1);
  }

  vga::wait_for_vblank();
  vga::video_on();

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

  // Because we copy, not flip, this can be hoisted out of loop.
  vga::Graphics1 g = rasterizer.make_bg_graphics();

  unsigned frame = 0;
  while (!user_button_pressed()) {
    ++frame;

    show_msg(frame);

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
    transform_vertices(m * m2);
    vga::msig_a_set();
    draw_edges(g);
    vga::msig_a_clear();

    vga::wait_for_vblank();
    rasterizer.copy_bg_to_fg();
  }

  vga::wait_for_vblank();
  vga::video_off();
  vga::configure_band_list(nullptr);
  vga::arena_reset();
}

}  // namespace rook
}  // namespace demo
