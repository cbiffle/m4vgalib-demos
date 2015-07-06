#include "demo/rook/rook.h"

#include "etl/assert.h"
#include "etl/math/affine.h"

#include "vga/arena.h"
#include "vga/graphics_1.h"
#include "vga/rasterizer.h"

#include "demo/rook/model.h"
#include "demo/input.h"
#include "demo/runner.h"

#include <cmath>

using etl::math::Mat4f;
using etl::math::Vec2i;
using etl::math::Vec3f;
using etl::math::Vec4f;

using math::Vec3h;

namespace demo {
namespace rook {

using Pixel = vga::Rasterizer::Pixel;


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

Wireframe::Wireframe()
    : transformed_vertices(vga::arena_new_array<Vec2i>(vertex_count)) {
  rasterizer.set_fg_color(0b111111);
  rasterizer.set_bg_color(0b010000);

  // Switch to whichever page has bitband support.
  if (!rasterizer.can_bg_use_bitband()) {
    rasterizer.flip_now();
    ETL_ASSERT(rasterizer.can_bg_use_bitband());
  }
}

Wireframe::~Wireframe() {
  transformed_vertices = nullptr;
}

constexpr Vec4f expand(Vec3h const & v) {
  return {v.x, v.y, v.z, 1};
}

constexpr Vec3f project(Vec4f v) {
  return {v.x/v.w, v.y/v.w, v.z/v.w};
}

__attribute__((section(".ramcode.transform_vertices")))
void Wireframe::transform_vertices(Mat4f const &m) const {
  for (unsigned i = 0; i < vertex_count; ++i) {
    Vec3f v = project(m * expand(vertices[i]));
    transformed_vertices[i] = { static_cast<int>(v.x),
                                static_cast<int>(v.y) };
  }
}

__attribute__((section(".ramcode.draw_edges")))
void Wireframe::draw_edges(vga::Graphics1 &g) {
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

BragLine::BragLine() {
  t_c = 0;
  string(0b111111, 0b010000, "2450 triangles - ");
  string(0b111111, 0b110000, "5.9Mpix/sec perspective projected fill rate");
  string(0b111111, 0b010000, " - ");
  string(0b000011, 0b010000, "60fps @ 800x600");
  string(0b111111, 0b010000, " - ");
}

void BragLine::string(Pixel fore, Pixel back, char const *s) {
  while (char c = *s++) {
    if (c == 0) break;
    message[t_c++] = (fore << 16) | (back << 8) | c;
  }
}

void BragLine::show_msg(unsigned frame) {
  auto pos_in_glyph = frame % 10;
  auto pos_in_message = (frame / 10) % 81;

  text.set_x_adj(-pos_in_glyph);

  text.clear_framebuffer(0b010000);
  for (unsigned i = 0; i < 81 - pos_in_message; ++i) {
    text.put_packed(i, 0, message[pos_in_message + i]);
  }
  for (unsigned i = 0; i < pos_in_message; ++i) {
    text.put_packed(81 - pos_in_message + i, 0, message[i]);
  }
}

/*******************************************************************************
 * The main bits.
 */

Rook::Rook()
  : _projection(
      etl::math::translate(Vec3f{config::cols/2, config::wireframe_rows/2, 0})
      * etl::math::scale(Vec3f{config::rows/2, config::rows/2, 1})
      * etl::math::persp(-10, -10, 10, 10, 20, 100)
      * etl::math::translate(Vec3f{0, 0, -70})),
    _model(Mat4f::identity()) {}

void Rook::configure_band_list() {
  vga::configure_band_list(_bands);
}

__attribute__((section(".ramcode.rook_run")))
bool Rook::render_frame(unsigned frame) {
  auto const continuing = !user_button_pressed();
  auto const j = read_joystick();
  _wireframe.rasterizer.copy_bg_to_fg();

  if (j & JoyBits::up) _model = _model * etl::math::rotate_z(-0.01f);
  if (j & JoyBits::down) _model = _model * etl::math::rotate_z(+0.01f);

  if (j & JoyBits::left) {
    _projection = _projection * etl::math::rotate_y(-0.01f);
  }
  if (j & JoyBits::right) {
    _projection = _projection * etl::math::rotate_y(+0.01f);
  }

  _brag_line.show_msg(frame % 810);

  auto g = _wireframe.rasterizer.make_bg_graphics();
  g.clear_all();
  _wireframe.transform_vertices(_projection * _model);
  _wireframe.draw_edges(g);

  return continuing;
}

void legacy_run() {
  vga::arena_reset();
  auto scene = vga::arena_make<Rook>();
  run_scene(*scene);

}

}  // namespace rook
}  // namespace demo
