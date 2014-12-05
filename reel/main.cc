#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/conway/conway.h"
#include "demo/tunnel/tunnel.h"
#include "demo/hires_mix/hires_mix.h"
#include "demo/hires_text/hires_text.h"
#include "demo/rook/rook.h"
#include "demo/wipe/wipe.h"
#include "demo/xor_pattern/xor.h"

int main() {
  vga::init();

  vga::configure_timing(vga::timing_vesa_800x600_60hz);

  while (true) {
    demo::hires_text::run();
    demo::hires_mix::run();
    demo::xor_pattern::run();
    demo::wipe::run();
    demo::tunnel::run();
    demo::conway::run(false);
    demo::rook::run();
  }
}
