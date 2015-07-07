#include "demo/raycast/raycast.h"

#include <cstdlib>
#include <cmath>

#include "etl/assert.h"
#include "etl/algorithm.h"
#include "etl/scope_guard.h"

#include "etl/math/matrix.h"

#include "math/conversion.h"

#include "vga/arena.h"
#include "vga/graphics_1.h"
#include "vga/measurement.h"
#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/input.h"
#include "demo/raycast/config.h"
#include "demo/raycast/map.h"
#include "demo/raycast/tex.h"
#include "demo/raycast/texture.h"

using etl::math::Mat2f;
using etl::math::Vec2f;
using etl::math::Vec2i;

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
  return canned_map.fetch(x, y);
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
  // The x value received here is in the range [-1, 1].  Multiply it by the
  // plane vector to displace the (camera) dir vector into a ray direction.
  auto dir = _dir + _plane * x;

  // map_pos gives our tile coordinate in the map.
  auto map_pos = Vec2i{math::floor(_pos.x), math::floor(_pos.y)};

  // The distance traveled by the ray for a move of one unit along either axis.
  // Note that for axis-aligned rays, the step distance along the other axis
  // becomes infinite.
  auto const delta_dist = Vec2f{
    sqrtf(1 + (dir.y * dir.y) / (dir.x * dir.x)),
    sqrtf(1 + (dir.x * dir.x) / (dir.y * dir.y)),
  };

  // side_dist is the distance along the ray for a move from _pos to the nearest
  // tile boundary along each axis.
  Vec2f side_dist;
  // step is the integer signum of dir.
  Vec2i step;

  if (dir.x < 0) {
    step.x = -1;
    side_dist.x = (_pos.x - map_pos.x) * delta_dist.x;
  } else {
    step.x = +1;
    side_dist.x = (map_pos.x + 1 - _pos.x) * delta_dist.x;
  }

  if (dir.y < 0) {
    step.y = -1;
    side_dist.y = (_pos.y - map_pos.y) * delta_dist.y;
  } else {
    step.y = +1;
    side_dist.y = (map_pos.y + 1 - _pos.y) * delta_dist.y;
  }

  // Step through the map until we find a cell containing a non-zero texture
  // number.  This produces the following outputs:
  // - map_pos will contain the location of the tile whose wall we hit.
  // - side will indicate whether the wall we hit was aligned with the X or Y
  //   axis.
  // - texnum will give the texture number.
  Hit::Side side;
  unsigned texnum;

  do {
    // If the ray needs to travel less far to reach an X-aligned wall than a
    // Y-aligned wall...
    if (side_dist.x < side_dist.y) {
      // ...then advance the ray along X.
      // side_dist now records the distance to the *next* X-aligned boundary.
      side_dist.x += delta_dist.x;
      // update map_pos by one.
      map_pos.x += step.x;
      // In case this is a hit, record the axis.
      side = Hit::Side::x;
    } else {
      // Otherwise, advance it along Y.
      // Same deal.
      side_dist.y += delta_dist.y;
      map_pos.y += step.y;
      side = Hit::Side::y;
    }

    texnum = map_fetch(map_pos.x, map_pos.y);
  } while (texnum == 0);

  // Decrement the texture number for zero-based texture array.
  // TODO: it is possible that using 0xFF for free space could be slightly more
  // efficient.
  texnum -= 1;

  // Common factor used by the two wall-collision equations below.  This gives
  // the distance along the ray to the wall we reached.
  //
  // This is 't' as in the parametric line equation.
  auto const t =
    (same(side, map_pos) - same(side, _pos) + (1 - same(side, step)) / 2)
        / same(side, dir);
  // TODO: can t be negative given its derivation above?  Is fabsf necessary
  // here?  Sure, it's a single cycle, but everything helps...
  auto const wall_dist = fabsf(t);
  // Location along the wall's axis where the ray hits, derived from t.  This
  // becomes the texture U coordinate.
  auto const wall_u = other(side, _pos) + t * other(side, dir);

  // The fractional part of wall_u gives us the U coordinate within the tile,
  // and thus the texture.  (Range: [0, 1) )
  auto const tile_u = wall_u - math::floor(wall_u);

  auto tex_u = unsigned(tile_u * config::tex_width);
  // The derivation of tex_u above is only a function of coordinate.  This looks
  // wrong on half of the walls: the texture appears mirror-imaged on two sides
  // of the walls.
  //
  // TODO: I don't understand why the axes are flipped here, with respect to
  // one another.  Do we have a mismatch in e.g. the UV coordinate frame?
  if ((side == Hit::Side::x && dir.x > 0)
      || (side == Hit::Side::y && dir.y < 0)) {
    tex_u = config::tex_width - tex_u - 1;
  }

  return {
    .texture = texnum,
    .tex_u = tex_u,
    .distance = wall_dist,
    .side = side,
  };
}

bool RayCast::render_frame(unsigned frame) {
  _rasterizer.flip_now();
  update_camera();

  auto const fb = _rasterizer.get_bg_buffer();

  // Produce pixels in vertical columns, once for each X coordinate of the
  // display.
  for (unsigned x = 0; x < config::cols; ++x) {
    // Convert the integer x coordinate to the range (-1, 1) to simplify the
    // casting math.
    auto const fx = 2 * x / float(config::cols) - 1;
    // Figure out where in the map we hit.  Note that a hit is guaranteed: the
    // map is closed (or is assumed to be closed).
    auto const hit = cast(fx);

    // Given the distance of the hit, apply simple perspective projection to
    // find the height of the textured pixel column we need to draw.
    // TODO: int(std::abs(x)) may actually be faster
    auto const col_height = std::abs(int(config::rows / hit.distance));
    // Get the Y coordinate of the first pixel drawn, limiting it to the top of
    // the display.
    auto const top = etl::max(-col_height / 2 + config::rows / 2, 0);

    vga::msig_e_set(1);

    // Draw the ceiling/floor.  This formulation of the loop seems to generate
    // the most efficient code on GCC 4.8.3.
    for (auto fill = &fb[x];
         fill != &fb[top * config::cols + x];
         fill += config::cols) {
      *fill = 0;
    }

    // For each y coordinate between top and the middle of the screen, the
    // tex_y value should be
    //
    //   tex_y = y * m + b
    //
    // for the values of m and b given below.
    //
    // However, we aren't going to write that in the loop below.  Instead, we'll
    // manually strength-reduce the formula by changing the loop to repeated
    // adds.
    //
    // This might seem counter-intuitive, since the M4 has a fused multiply add
    // operation that can evaluate the full equation nearly as fast as an add.
    // However, using that really pessimizes GCC 4.8.3's output.  The loop
    // gains a lot of added fat and becomes 30% or so slower.
    auto const m = float(config::apparent_tex_height) / col_height;
    auto const b = (-config::rows / 2.f + col_height / 2.f) * m;
    auto tex_y = top * m + b;

    if (hit.side == Hit::Side::y) {
      // Darken the texture slightly (Wolfenstein-style)
      for (unsigned y = top; y < config::rows/2; ++y) {
        fb[y * config::cols + x] = tex_darken[
          tex_tex[hit.texture].fetch(hit.tex_u, int(tex_y))];
        tex_y += m;
      }
    } else {
      // Render the texture faithfully.
      for (unsigned y = top; y < config::rows/2; ++y) {
        fb[y * config::cols + x] =
          tex_tex[hit.texture].fetch(hit.tex_u, int(tex_y));
        tex_y += m;
      }
    }

    vga::msig_e_clear(1);
  }

  return true;
}

void RayCast::update_camera() {
  auto const j = read_joystick();

  if (j & JoyBits::up)   move(_dir * +0.1f);
  if (j & JoyBits::down) move(_dir * -0.1f);

  if (j & JoyBits::left) rotate(+0.01f);
  if (j & JoyBits::right) rotate(-0.01f);
}

void RayCast::rotate(float a) {
  auto const m = Mat2f {
    { cosf(a), -sinf(a) },
    { sinf(a), cosf(a) },
  };
  _dir = m * _dir;
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
