#include "demo/raycast/raycast.h"

#include <cstdlib>
#include <cmath>

#include "etl/assert.h"
#include "etl/algorithm.h"
#include "etl/scope_guard.h"

#include "math/conversion.h"

#include "vga/arena.h"
#include "vga/graphics_1.h"
#include "vga/measurement.h"
#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/input.h"
#include "demo/raycast/config.h"
#include "demo/raycast/texture.h"
#include "demo/raycast/tex.h"

using math::Mat2f;
using math::Vec2f;
using math::Vec2i;

namespace demo {
namespace raycast {

using vga::Pixel;

RayCast::RayCast()
  : _pos{10, 10},
    _dir{-1, 0},
    _plane{0, config::fov} {
  auto fb = _rasterizer.get_fg_buffer();
  for (unsigned y = 0; y < config::rows/2; ++y) {
    for (unsigned x = 0; x < config::cols; ++x) {
      fb[y * config::cols + x] = x;
    }
  }

  for (unsigned i = 0; i < tex_color_count; ++i) {
    _rasterizer.get_palette()[i] = tex_palette_top[i];
    _mirror.get_palette()[i] = tex_palette_bot[i];
  }

  _mirror.get_palette()[0] = 0b000100;
}

void RayCast::configure_band_list() {
  vga::configure_band_list(_bands);
}

static unsigned map_fetch(int x, int y) {
  if (std::abs(x - 10) > 2) return 2;
  if (std::abs(y - 10) > 10) return x & 1 ? 1 : 3;
  return 0;
}

static int same(Hit::Side side, Vec2i v) {
  return side == Hit::Side::x ? v.x : v.y;
}

static float same(Hit::Side side, Vec2f v) {
  return side == Hit::Side::x ? v.x : v.y;
}

static float other(Hit::Side side, Vec2f v) {
  return side == Hit::Side::x ? v.y : v.x;
}

Hit RayCast::cast(float x) const {
  auto const ray_pos = _pos;
  auto const ray_dir = _dir + _plane * x;

  auto map_pos = Vec2i{math::floor(ray_pos.x), math::floor(ray_pos.y)};

  auto const delta_dist = Vec2f{
    sqrtf(1 + (ray_dir.y * ray_dir.y) / (ray_dir.x * ray_dir.x)),
    sqrtf(1 + (ray_dir.x * ray_dir.x) / (ray_dir.y * ray_dir.y)),
  };

  Vec2f side_dist;
  Vec2i step;

  if (ray_dir.x < 0) {
    step.x = -1;
    side_dist.x = (ray_pos.x - map_pos.x) * delta_dist.x;
  } else {
    step.x = +1;
    side_dist.x = (map_pos.x + 1 - ray_pos.x) * delta_dist.x;
  }

  if (ray_dir.y < 0) {
    step.y = -1;
    side_dist.y = (ray_pos.y - map_pos.y) * delta_dist.y;
  } else {
    step.y = +1;
    side_dist.y = (map_pos.y + 1 - ray_pos.y) * delta_dist.y;
  }

  Hit::Side side;
  unsigned texnum;

  do {
    if (side_dist.x < side_dist.y) {
      side_dist.x += delta_dist.x;
      map_pos.x += step.x;
      side = Hit::Side::x;
    } else {
      side_dist.y += delta_dist.y;
      map_pos.y += step.y;
      side = Hit::Side::y;
    }

    texnum = map_fetch(map_pos.x, map_pos.y);
  } while (texnum == 0);

  texnum -= 1;

  auto const t =
    (same(side, map_pos) - same(side, ray_pos) + (1 - same(side, step)) / 2)
        / same(side, ray_dir);
  auto const wall_dist = fabsf(t);
  auto const wall_x = other(side, ray_pos) + t * other(side, ray_dir);

  auto const tile_x = wall_x - math::floor(wall_x);

  auto tex_x = unsigned(tile_x * config::tex_width);
  if (same(side, ray_dir) > 0) {
    tex_x = config::tex_width - tex_x - 1;
  }

  return {
    .texture = texnum,
    .tex_x = tex_x,
    .distance = wall_dist,
    .side = side,
  };
}

bool RayCast::render_frame(unsigned frame) {
  _rasterizer.flip_now();
  update_camera();

  auto fb = _rasterizer.get_bg_buffer();

  for (unsigned x = 0; x < config::cols; ++x) {
    auto const fx = 2 * x / float(config::cols) - 1;
    auto const hit = cast(fx);

    auto const line_height = std::abs(int(config::rows / hit.distance));
    auto const top = etl::max(-line_height / 2 + config::rows / 2, 0);

    vga::msig_e_set(1);
    // Draw the ceiling, floor.
    for (auto fill = &fb[x];
         fill != &fb[top * config::cols + x];
         fill += config::cols) {
      *fill = 0;
    }

    auto const m = float(config::apparent_tex_height) / line_height;
    auto const b = (-config::rows / 2.f + line_height / 2.f) * m;

    auto tex_y = top * m + b;

    for (unsigned y = top; y < config::rows/2; ++y) {
      fb[y * config::cols + x] =
          tex_tex[hit.texture].fetch(hit.tex_x, int(tex_y));
      tex_y += m;
    }

    vga::msig_e_clear(1);
  }

  return true;
}

void RayCast::update_camera() {
  auto const j = read_joystick();

  if (j & JoyBits::up) move(_dir * +0.1f);
  if (j & JoyBits::down) move(_dir * -0.1f);

  if (j & JoyBits::left) rotate(+0.01f);
  if (j & JoyBits::right) rotate(-0.01f);
}

void RayCast::rotate(float a) {
  _dir = Mat2f::rotate(a) * _dir;
  _plane = Vec2f{_dir.y, -_dir.x} * config::fov;
}

void RayCast::move(Vec2f delta) {
  auto new_pos = _pos + delta;
  if (map_fetch(math::floor(new_pos.x), math::floor(new_pos.y)) == 0) {
    _pos = new_pos;
  }
}

}  // namespace raycast
}  // namespace demo
