#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/rotozoom/rotozoom.h"

int main() {
  vga::init();
  vga::configure_timing(vga::timing_vesa_800x600_60hz);

  while (true) {
    demo::rotozoom::run();
  }
}

