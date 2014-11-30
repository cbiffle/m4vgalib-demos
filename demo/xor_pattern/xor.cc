#include "demo/xor_pattern/xor.h"

#include "etl/scope_guard.h"
#include "etl/armv7m/instructions.h"

#include "vga/arena.h"
#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/xor_pattern/rasterizer.h"

namespace demo {
namespace xor_pattern {

struct Demo {
  Rasterizer rasterizer{vga::timing_vesa_800x600_60hz};
  vga::Band const band{&rasterizer, 600, nullptr};
};

void run(unsigned frame_count) {
  vga::arena_reset();

  auto d = vga::arena_make<Demo>();

  vga::configure_band_list(&d->band);
  ETL_ON_SCOPE_EXIT { vga::clear_band_list(); };

  vga::video_on();

  unsigned frame = 0;
  while (frame_count == 0 || frame++ < frame_count) {
    vga::sync_to_vblank();
  }

  vga::wait_for_vblank();
  vga::video_off();
}

}  // namespace xor_pattern
}  // namespace demo
