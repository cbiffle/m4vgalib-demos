#include "etl/armv7m/exception_table.h"

#include "runtime/crt.h"

#include "vga/timing.h"
#include "vga/vga.h"
#include "demo/conway/conway.h"

void etl_armv7m_reset_handler() {
  crt_init();
  vga::init();
  vga::configure_timing(vga::timing_vesa_800x600_60hz);

  while (true) {
    demo::conway::run_demo(10 * 60, true);
  }
}
