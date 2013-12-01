#include "lib/armv7m/crt0.h"
#include "lib/armv7m/exception_table.h"
#include "lib/armv7m/instructions.h"
#include "lib/armv7m/scb.h"

#include "lib/stm32f4xx/rcc.h"
#include "lib/stm32f4xx/syscfg.h"

#include "vga/arena.h"
#include "vga/vga.h"

static stm32f4xx::ClockConfig const clock_cfg = {
  8000000,  // external crystal Hz
  8,        // divide down to 1Mhz
  320,      // multiply up to 320MHz VCO
  2,        // divide by 2 for 160MHz CPU clock
  7,        // divide by 7 for 48MHz-ish SDIO clock
  1,        // divide CPU clock by 1 for 160MHz AHB clock
  4,        // divide CPU clock by 4 for 40MHz APB1 clock.
  2,        // divide CPU clock by 2 for 80MHz APB2 clock.

  5,        // 5 wait states for 160MHz at 3.3V.
};

static vga::VideoMode const mode = {
  &clock_cfg,

  1056,  // line_pixels
  128,   // sync_pixels
  88,    // back_porch_pixels
  22,    // video_lead
  800,   // video_pixels,
  vga::VideoMode::Polarity::positive,

  1,
  1 + 4,
  1 + 4 + 23,
  1 + 4 + 23 + 600,
  vga::VideoMode::Polarity::positive,
};

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

  vga::select_mode(mode);

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
