#include "etl/armv7m/implicit_crt0.h"

#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/hires_mix/hires_mix.h"

int main() {
  vga::init();
  vga::configure_timing(vga::timing_vesa_800x600_60hz);

  while (true) {
    demo::hires_mix::run();
  }
  __builtin_unreachable();
}
