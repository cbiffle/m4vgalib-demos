#include "runtime/ramcode.h"

#include "lib/armv7m/instructions.h"
#include "lib/armv7m/scb.h"

#include "lib/stm32f4xx/rcc.h"
#include "lib/stm32f4xx/syscfg.h"

using armv7m::scb;

using stm32f4xx::ApbPeripheral;
using stm32f4xx::rcc;
using stm32f4xx::SysCfg;
using stm32f4xx::syscfg;

extern "C" {
  /*
   * Symbols we use to communicate with the linker script.
   */
  extern unsigned _rom_vector_table_start;
  extern unsigned _rom_vector_table_end;
  extern unsigned _ram_vector_table;
  extern unsigned _flash_base;
  extern unsigned _ramcode_lma;
  extern unsigned _ramcode_start;
  extern unsigned _ramcode_end;
}

namespace runtime {

/*
 * Remap the 112KiB SRAM to address zero, so we can access it using the
 * Cortex-M4 I/D buses.
 */
static void remap_sram() {
  // Power on syscfg, so we can mess with its registers.
  rcc.enable_clock(ApbPeripheral::syscfg);

  // VTOR starts out at zero, which points to Flash and is good.
  // But now things are about to change.
  // Interrupts are disabled, but to be safe in case we fault (due to a bug),
  // go ahead and give VTOR the true address of the Flash table.
  scb.write_vtor(reinterpret_cast<unsigned>(&_flash_base));
  armv7m::data_synchronization_barrier();  // Write it now.
  armv7m::instruction_synchronization_barrier();  // Flush pipeline just in case

  // Remap!
  auto mode = SysCfg::memrmp_value_t::mem_mode_t::embedded_sram;
  syscfg.write_memrmp(syscfg.read_memrmp()
                      .with_mem_mode(mode));
  armv7m::data_synchronization_barrier();  // Write it now.
  armv7m::instruction_synchronization_barrier();  // Flush pipeline just in case
}

/*
 * Copy the vector table into SRAM to reduce interrupt jitter.
 */
static void copy_vector_table() {
  unsigned const *src = &_rom_vector_table_start;
  unsigned const *end = &_rom_vector_table_end;

  unsigned *dst = &_ram_vector_table;

  while (src != end) {
    *dst++ = *src++;
  }

  scb.write_vtor(reinterpret_cast<unsigned>(&_ram_vector_table));
  armv7m::data_synchronization_barrier();  // Write it now.
  armv7m::instruction_synchronization_barrier();  // Flush pipeline just in case
}

/*
 * Copy selected routines into SRAM to remove wait states from instruction
 * fetches and constant loads.
 */
static void copy_ramcode() {
  unsigned const *src = &_ramcode_lma;
  unsigned *dst = &_ramcode_start;
  unsigned const *end = &_ramcode_end;

  while (dst != end) {
    *dst++ = *src++;
  }
  armv7m::data_synchronization_barrier();  // Ensure writes complete...
  armv7m::instruction_synchronization_barrier();  // ...before we fetch more.
}

void ramcode_init() {
  remap_sram();
  copy_vector_table();
  copy_ramcode();
}

}  // namespace runtime
