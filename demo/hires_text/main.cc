#include "etl/armv7m/exception_table.h"

#include "etl/armv7m/crt0.h"

#include "demo/hires_text/hires_text.h"
#include "vga/timing.h"
#include "vga/vga.h"

__attribute__((noinline))
static void rest() {
  vga::init();
  vga::configure_timing(vga::timing_vesa_800x600_60hz);

  while (true) {
    demo::hires_text::run();
  }
}

void etl_armv7m_reset_handler() {
  etl::armv7m::crt0_init();
  rest();
}
