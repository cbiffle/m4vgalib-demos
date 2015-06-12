#include "demo/wipe/wipe.h"

#include <cmath>

#include "vga/arena.h"

#include "demo/input.h"
#include "demo/runner.h"

namespace demo {
namespace wipe {

static constexpr char message[82] =
// 12345678901234567890
  "Dynamic Mixed-Mode G"
  "raphics  -  any numb"
  "er  -  separate dept"
  "h and resolution  - "
  " ";

void Wipe::configure_band_list() {
  vga::configure_band_list(_bands);
}

bool Wipe::render_frame(unsigned frame) {
  bool const continuing = !user_button_pressed();

  auto center_height =
      unsigned((std::sin(float(frame) / 50) + 1) * (config::max_band_height/2));

  auto split = config::rows/2 +
      std::sin(float(frame) / 127) * (config::rows/4);

  auto scroll = frame * 2;

  _bands[0].line_count = split - center_height/2;
  _bands[1].line_count = center_height;
  _bands[2].line_count = config::rows - (split + center_height/2);

  _term.rasterizer.set_top_line(split - config::max_band_height/2);
  _term.rasterizer.set_x_adj(-(scroll % 10));

  unsigned x = (scroll / 10) % 81;
  _term.cursor_to(0, 1);
  for (unsigned i = 0; i < 81 - x; ++i) {
    _term.type_raw(demo::white, demo::black, message[x + i]);
  }
  for (unsigned i = 0; i < x; ++i) {
    _term.type_raw(demo::white, demo::black, message[i]);
  }

  return continuing;
}

void legacy_run() {
  vga::arena_reset();
  auto scene = vga::arena_make<Wipe>();
  run_scene(*scene);
}

}  // namespace wipe
}  // namespace demo
