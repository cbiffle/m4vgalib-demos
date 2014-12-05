#include "demo/wipe/wipe.h"

#include <cmath>

#include "etl/scope_guard.h"

#include "etl/armv7m/instructions.h"
#include "etl/armv7m/types.h"

#include "vga/arena.h"
#include "vga/measurement.h"
#include "vga/timing.h"
#include "vga/vga.h"
#include "vga/rast/solid_color.h"

#include "math/geometry.h"
#include "math/interpolate.h"

#include "demo/input.h"
#include "demo/terminal.h"
#include "demo/wipe/config.h"

#include "demo/xor_pattern/rasterizer.h"

namespace demo {
namespace wipe {

constexpr unsigned max_band_height = 3 * 16;

/*
 * Demo state.
 */
struct State {
  demo::xor_pattern::Rasterizer border { config::cols };
  demo::Terminal term { config::cols + 10, max_band_height,
                        config::rows/2 - max_band_height/2 };

  vga::Band bands[3] {
    { &border,          config::rows/2, &bands[1] },
    { &term.rasterizer, 0,              &bands[2] },
    { &border,          config::rows/2, nullptr },
  };
};

constexpr char message[82] =
// 12345678901234567890
  "Dynamic Mixed-Mode G"
  "raphics  -  any numb"
  "er  -  separate dept"
  "h and resolution  - "
  " ";

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

  vga::configure_band_list(d->bands);
  ETL_ON_SCOPE_EXIT { vga::clear_band_list(); };

  unsigned frame = 0;
  unsigned split = config::rows/2;
  while (!user_button_pressed()) {
    ++frame;

    auto center_height =
        unsigned((std::sin(float(frame) / 50) + 1) * (max_band_height/2));

    split = config::rows/2 +
        std::sin(float(frame) / 127) * (config::rows/4);

    auto scroll = frame * 2;

    vga::msig_a_clear();
    vga::sync_to_vblank();
    d->bands[0].line_count = split - center_height/2;
    d->bands[1].line_count = center_height;
    d->bands[2].line_count = config::rows - (split + center_height/2);

    d->term.rasterizer.set_top_line(split - max_band_height/2);
    d->term.rasterizer.set_x_adj(-(scroll % 10));

    unsigned x = (scroll / 10) % 81;
    d->term.cursor_to(0, 1);
    for (unsigned i = 0; i < 81 - x; ++i) {
      d->term.type_raw(demo::white, demo::black, message[x + i]);
    }
    for (unsigned i = 0; i < x; ++i) {
      d->term.type_raw(demo::white, demo::black, message[i]);
    }

    if (!video_on) {
      vga::video_on();
      video_on = true;
    }
    vga::msig_a_set();
  }
}

}  // namespace wipe
}  // namespace demo
