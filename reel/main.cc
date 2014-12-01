#include "etl/armv7m/exception_table.h"

#include "etl/armv7m/crt0.h"

#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/conway/conway.h"
#include "demo/tunnel/tunnel.h"
#include "demo/hires_mix/hires_mix.h"
#include "demo/hires_text/hires_text.h"
#include "demo/rook/rook.h"
#include "demo/xor_pattern/xor.h"

__attribute__((noreturn))
__attribute__((noinline))
static void rest() {
  vga::init();

  vga::configure_timing(vga::timing_vesa_800x600_60hz);

  while (1) {
    demo::hires_text::run();
    demo::hires_mix::run();
    demo::rook::run();
    demo::conway::run(false);
    demo::xor_pattern::run();
    demo::tunnel::run();
  }
}

__attribute__((noreturn))
void etl_armv7m_reset_handler() {
  etl::armv7m::crt0_init();
  rest();
}
