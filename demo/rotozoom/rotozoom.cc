#include "demo/rotozoom/rotozoom.h"

#include <cmath>

#include "etl/scope_guard.h"

#include "etl/armv7m/instructions.h"
#include "etl/armv7m/types.h"

#include "etl/math/affine_transform.h"
#include "etl/math/matrix.h"
#include "etl/math/vector.h"

#include "vga/arena.h"
#include "vga/measurement.h"
#include "vga/timing.h"
#include "vga/vga.h"
#include "vga/rast/direct.h"

#include "math/interpolate.h"

#include "demo/input.h"
#include "demo/rotozoom/config.h"

using etl::math::Mat3f;

using etl::math::Vec2i;
using etl::math::Vec2f;
using etl::math::Vec3f;

namespace xf = etl::math::affine_transform;

namespace demo {
namespace rotozoom {

/*
 * Demo state.
 */
struct State {
  vga::rast::Direct rasterizer{
    config::cols * 4, config::rows * 4,
    4, 4,
  };
  vga::Band band { &rasterizer, config::rows * 4, nullptr };
};

static constexpr auto center = Vec2i { config::cols/2, config::rows/2 };

/*
 * Entry point.
 */
void run() {
  vga::arena_reset();
  vga::msigs_init();
  input_init();

  auto d = vga::arena_make<State>();

  bool video_on = false;
  ETL_ON_SCOPE_EXIT { if (video_on) vga::video_off(); };

  vga::configure_band_list(&d->band);
  ETL_ON_SCOPE_EXIT { vga::clear_band_list(); };

  auto m = Mat3f::identity();
  unsigned frame = 0;
  while (!user_button_pressed()) {
    ++frame;

    float s = std::sin(float(frame) / 63) + 1.3f;
    float tx = std::cos(float(frame) / 59) * 50;
    float ty = std::sin(float(frame) / 50) * 50;

    auto const scale = xf::scale(Vec2f{s, s});
    auto const trans = xf::translate(Vec2f{tx, ty});

    auto const m_ = m * trans * scale;

    Vec2f const vertices[4] {
      xf::project(m_ * xf::augment(Vec2f{-config::cols/2, -config::rows/2})),
      xf::project(m_ * xf::augment(Vec2f{+config::cols/2, -config::rows/2})),
      xf::project(m_ * xf::augment(Vec2f{-config::cols/2, +config::rows/2})),
      xf::project(m_ * xf::augment(Vec2f{+config::cols/2, +config::rows/2})),
    };

    auto xi = (vertices[1] - vertices[0]) * (1.f / config::cols);

    uint8_t * fb = d->rasterizer.get_bg_buffer();
    for (unsigned y = 0; y < config::rows; ++y) {
      float yr = float(y) / config::rows;

      auto pos = math::linear_interpolate(vertices[0], vertices[2], yr);

      for (unsigned x = 0; x < config::cols; ++x) {
        fb[y * config::cols + x] = unsigned(int(pos.x))
                                 ^ unsigned(int(pos.y));
        pos = pos + xi;
      }
    }

    auto const r = xf::rotate(0.01f);

    m = m * r;

    vga::msig_a_clear();
    vga::sync_to_vblank();
    d->rasterizer.flip_now();
    if (!video_on) {
      vga::video_on();
      video_on = true;
    }
    vga::msig_a_set();
  }
}

}  // namespace rotozoom
}  // namespace demo
