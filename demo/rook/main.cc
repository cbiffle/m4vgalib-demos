#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/rook/rook.h"

int main() {
  vga::init();

  vga::configure_timing(vga::timing_vesa_800x600_60hz);

  while (1) {
    demo::rook::run();
  }
}
