#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/wipe/wipe.h"

int main() {
  vga::init();
  vga::configure_timing(vga::timing_vesa_800x600_60hz);

  while (true) {
    demo::wipe::run();
  }
}

