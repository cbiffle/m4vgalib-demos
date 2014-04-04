#include "etl/armv7m/exception_table.h"

#include "runtime/startup.h"

#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/rook/rook.h"

__attribute__((noreturn))
__attribute__((noinline))
static void rest() {
  crt_init();
  vga::init();

  vga::configure_timing(vga::timing_vesa_800x600_60hz);

  while (1) {
    demo::rook::run(60 * 30);
  }
}

__attribute__((noreturn))
void etl_armv7m_reset_handler() {
  crt_init();
  rest();
}
