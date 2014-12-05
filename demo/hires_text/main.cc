#include "etl/armv7m/implicit_crt0.h"

#include "demo/hires_text/hires_text.h"
#include "vga/timing.h"
#include "vga/vga.h"

int main() {
  vga::init();
  vga::configure_timing(vga::timing_vesa_800x600_60hz);

  while (true) {
    demo::hires_text::run();
  }
  __builtin_unreachable();
}
