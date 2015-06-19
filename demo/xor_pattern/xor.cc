#include "demo/xor_pattern/xor.h"

#include "etl/scope_guard.h"
#include "etl/armv7m/instructions.h"

#include "vga/arena.h"
#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/config.h"
#include "demo/input.h"
#include "demo/xor_pattern/rasterizer.h"

DEMO_REQUIRE_RESOLUTION(800, 600)

namespace demo {
namespace xor_pattern {

struct Demo {
  Rasterizer rasterizer{800};
  vga::Band const band{&rasterizer, 600, nullptr};
};

void run() {
  vga::arena_reset();

  input_init();

  auto d = vga::arena_make<Demo>();

  vga::configure_band_list(&d->band);
  ETL_ON_SCOPE_EXIT { vga::clear_band_list(); };

  vga::video_on();

  while (!user_button_pressed()) {
    vga::sync_to_vblank();
  }

  vga::wait_for_vblank();
  vga::video_off();
}

}  // namespace xor_pattern
}  // namespace demo
