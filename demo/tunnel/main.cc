#include "etl/concatenate.h"

#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/tunnel/tunnel.h"

// I hate the C preprocessor.
#define MODE_(w, h) vga::timing_vesa_ ## w ## x ## h ## _60hz
#define MODE(w, h) MODE_(w, h)

int main() {
  vga::init();
  vga::configure_timing(MODE(CFG_WIDTH, CFG_HEIGHT));

  while (true) {
    demo::tunnel::run();
  }
}

