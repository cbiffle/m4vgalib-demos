#include "demo/xor_pattern/xor.h"

#include "lib/armv7m/instructions.h"

#include "vga/arena.h"
#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/xor_pattern/rasterizer.h"

namespace demo {
namespace xor_pattern {

static demo::xor_pattern::Rasterizer rasterizer;
static vga::Band const band = { &rasterizer, 600, nullptr };

void run(unsigned frame_count) {
  rasterizer.activate(vga::timing_vesa_800x600_60hz);
  vga::configure_band_list(&band);
  vga::video_on();

  unsigned frame = 0;
  while (frame_count == 0 || frame++ < frame_count) {
    vga::sync_to_vblank();
  }

  vga::configure_band_list(nullptr);
  vga::wait_for_vblank();
  vga::video_off();
  vga::arena_reset();
}

}  // namespace xor_pattern
}  // namespace demo
