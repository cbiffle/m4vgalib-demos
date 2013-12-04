#include "demo/xor_pattern/mode_800x600.h"

#include "lib/stm32f4xx/rcc.h"
#include "vga/timing.h"

namespace demo {
namespace xor_pattern {

static constexpr unsigned output_width = 800;

static stm32f4xx::ClockConfig const clock_cfg = {
  8000000,  // external crystal Hz
  8,        // divide down to 1Mhz
  320,      // multiply up to 320MHz VCO
  2,        // divide by 2 for 160MHz CPU clock
  7,        // divide by 7 for 48MHz-ish SDIO clock
  1,        // divide CPU clock by 1 for 160MHz AHB clock
  4,        // divide CPU clock by 4 for 40MHz APB1 clock.
  2,        // divide CPU clock by 2 for 80MHz APB2 clock.

  5,        // 5 wait states for 160MHz at 3.3V.
};

// We make this non-const to force it into RAM, because it's accessed from
// paths that won't enjoy the Flash wait states.
static vga::Timing timing = {
  &clock_cfg,

  1056,  // line_pixels
  128,   // sync_pixels
  88,    // back_porch_pixels
  22,    // video_lead
  800,   // video_pixels,
  vga::Timing::Polarity::positive,

  1,
  1 + 4,
  1 + 4 + 23,
  1 + 4 + 23 + 600,
  vga::Timing::Polarity::positive,
};

void Mode_800x600::activate() {
  _rr.activate(timing);
}

__attribute__((section(".ramcode")))
void Mode_800x600::rasterize(unsigned line_number, Pixel *target) {
  line_number -= timing.video_start_line;
  (void) _rr.rasterize(line_number, target);
}

__attribute__((section(".ramcode")))
vga::Timing const &Mode_800x600::get_timing() const {
  return timing;
}

}  // namespace xor_pattern
}  // namespace demo
