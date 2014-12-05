#include "etl/armv7m/implicit_crt0.h"

#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/xor_pattern/xor.h"

int main() {
  vga::init();
  vga::configure_timing(vga::timing_vesa_800x600_60hz);

  while (true) {
    demo::xor_pattern::run();
  }
  __builtin_unreachable();
}
