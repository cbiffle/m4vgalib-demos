#include "lib/armv7m/crt0.h"
#include "lib/armv7m/exception_table.h"
#include "lib/armv7m/instructions.h"
#include "lib/armv7m/scb.h"

#include "lib/stm32f4xx/rcc.h"
#include "lib/stm32f4xx/syscfg.h"

#include "vga/arena.h"
#include "vga/vga.h"
#include "vga/mode/raster_640x480x1.h"
#include "vga/mode/raster_800x600x1.h"
#include "vga/mode/text_800x600.h"

extern "C" {
  extern unsigned _rom_vector_table_start;
  extern unsigned _rom_vector_table_end;
  extern unsigned _ram_vector_table;
  extern unsigned _flash_base;
  extern unsigned _ramcode_lma;
  extern unsigned _ramcode_start;
  extern unsigned _ramcode_end;
}

/*
 * Remap the 112KiB SRAM to address zero, so we can access it using the
 * Cortex-M4 I/D buses.
 */
static void remap_sram() {
  // Power on syscfg, so we can mess with its registers.
  stm32f4xx::rcc.enable_clock(stm32f4xx::ApbPeripheral::syscfg);

  // VTOR starts out at zero, which points to Flash and is good.
  // But now things are about to change.
  // Interrupts are disabled, but to be safe in case we fault (due to a bug),
  // go ahead and give VTOR the true address of the Flash table.
  armv7m::scb.write_vtor(reinterpret_cast<unsigned>(&_flash_base));
  armv7m::data_synchronization_barrier();  // Write it now.
  armv7m::instruction_synchronization_barrier();  // Flush pipeline just in case

  // Remap!
  auto mode = stm32f4xx::SysCfg::memrmp_value_t::mem_mode_t::embedded_sram;
  stm32f4xx::syscfg.write_memrmp(stm32f4xx::syscfg.read_memrmp()
                                 .with_mem_mode(mode));
  armv7m::data_synchronization_barrier();  // Write it now.
  armv7m::instruction_synchronization_barrier();  // Flush pipeline just in case
}

/*
 * Move the vector table into SRAM to reduce interrupt jitter.
 */
static void move_vector_table() {
  unsigned const *src = &_rom_vector_table_start;
  unsigned const *end = &_rom_vector_table_end;

  unsigned *dst = &_ram_vector_table;

  while (src != end) {
    *dst++ = *src++;
  }

  armv7m::scb.write_vtor(reinterpret_cast<unsigned>(&_ram_vector_table));
}

/*
 * Move selected routines into SRAM to remove wait states from instruction
 * fetches and constant loads.
 */
static void move_ramcode() {
  unsigned const *src = &_ramcode_lma;
  unsigned *dst = &_ramcode_start;
  unsigned const *end = &_ramcode_end;

  while (dst != end) {
    *dst++ = *src++;
  }
}

static vga::mode::Raster_640x480x1 raster_640;
static vga::mode::Raster_800x600x1 raster_800;
static vga::mode::Text_800x600 text_800;

void v7m_reset_handler() {
  armv7m::crt0_init();

  remap_sram();
  move_vector_table();
  move_ramcode();

  // Enable fault reporting.
  armv7m::scb.write_shcsr(armv7m::scb.read_shcsr()
                          .with_memfaultena(true)
                          .with_busfaultena(true)
                          .with_usgfaultena(true));

  // Enable floating point automatic/lazy state preservation.
  // The CONTROL bit governing FP will be set automatically when first used.
  armv7m::scb_fp.write_fpccr(armv7m::scb_fp.read_fpccr()
                             .with_aspen(true)
                             .with_lspen(true));
  armv7m::instruction_synchronization_barrier();  // Now please.

  // Enable access to the floating point coprocessor.
  armv7m::scb.write_cpacr(armv7m::scb.read_cpacr()
                          .with_cp11(armv7m::Scb::CpAccess::full)
                          .with_cp10(armv7m::Scb::CpAccess::full));

  // It is now safe to use floating point.

  vga::arena_reset();

  vga::init();

  while (1) {
    vga::select_mode(&raster_800);
    for (volatile unsigned i = 0; i < 50000000; ++i);
    vga::select_mode(&raster_640);
    for (volatile unsigned i = 0; i < 50000000; ++i);
    vga::select_mode(&text_800);
    for (volatile unsigned i = 0; i < 50000000; ++i);
  }

  while (1);
}


/*
 * Catch unexpected exceptions in a loop.
 */
#define TRAP(name) \
  void v7m_ ## name ## _handler() { \
    while (1); \
  }

TRAP(nmi)
TRAP(hard_fault)
TRAP(mem_manage_fault)
TRAP(bus_fault)
TRAP(usage_fault)
TRAP(sv_call)
TRAP(debug_monitor)
TRAP(sys_tick)
