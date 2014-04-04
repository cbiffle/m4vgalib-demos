/*
 * This provides specialized C runtime startup for our memory map.
 */

#include "runtime/startup.h"

#include "etl/common/attribute_macros.h"

#include "etl/armv7m/types.h"
#include "lib/armv7m/instructions.h"
#include "lib/armv7m/scb.h"

#include "lib/stm32f4xx/rcc.h"
#include "lib/stm32f4xx/syscfg.h"

using etl::armv7m::Word;

using armv7m::scb;

using stm32f4xx::ApbPeripheral;
using stm32f4xx::rcc;
using stm32f4xx::SysCfg;
using stm32f4xx::syscfg;

typedef void (*InitFnPtr)();

/*
 * These symbols are provided by the linker script.
 */
extern "C" {
  // Start of image to be copied.
  extern Word _sram_image_start;
  // End of image to be copied.
  extern Word _sram_image_end;
  // Destination for copy (RAM).
  extern Word _sram_image_dest;

  // Start/end of memory to be zeroed.
  extern Word _bss_start, _bss_end;

  // Start/end of initializer arrays.
  extern InitFnPtr _preinit_array_start, _preinit_array_end;
  extern InitFnPtr _init_array_start, _init_array_end;

  // Generated initializer function.
  extern void _init();

  // Init epilogue, defined below, called implicitly.
  void init_epilogue();
}

void crt_init() {
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

  // Enable access to the floating point coprocessor.
  armv7m::scb.write_cpacr(armv7m::scb.read_cpacr()
                          .with_cp11(armv7m::Scb::CpAccess::full)
                          .with_cp10(armv7m::Scb::CpAccess::full));
  armv7m::instruction_synchronization_barrier();  // Now please.

  // It is now safe to use floating point.

  // Remap SRAM.
  // Power on syscfg, so we can mess with its registers.
  rcc.enable_clock(ApbPeripheral::syscfg);

  // VTOR starts out at zero, which points to Flash and is good.
  // But now things are about to change.
  // Interrupts are disabled, but to be safe in case we fault (due to a bug),
  // go ahead and give VTOR the true address of the Flash table.
  scb.write_vtor(reinterpret_cast<unsigned>(&_sram_image_dest));
  armv7m::data_synchronization_barrier();  // Write it now.
  armv7m::instruction_synchronization_barrier();  // Flush pipeline just in case

  // Remap!
  auto mode = SysCfg::memrmp_value_t::mem_mode_t::embedded_sram;
  syscfg.write_memrmp(syscfg.read_memrmp()
                      .with_mem_mode(mode));
  armv7m::data_synchronization_barrier();  // Write it now.
  armv7m::instruction_synchronization_barrier();  // Flush pipeline just in cas

  // Copy image into SRAM.
  for (Word *src = &_sram_image_start, *dest = &_sram_image_dest;
       src != &_sram_image_end;
       ++src, ++dest) {
    *dest = *src;
  }

  // Zero BSS.
  for (Word *dest = &_bss_start; dest != &_bss_end; ++dest) {
    *dest = 0;
  }

  // Run the funky three-phase init process.
  for (InitFnPtr *f = &_preinit_array_start; f != &_preinit_array_end; ++f) {
    (*f)();
  }

  _init();

  for (InitFnPtr *f = &_init_array_start; f != &_init_array_end; ++f) {
    (*f)();
  }
}

ETL_SECTION(".init_prologue")
ETL_NAKED void _init() {
  asm volatile ("push {r4-r11, lr}");
}

ETL_SECTION(".init_epilogue")
ETL_NAKED void init_epilogue() {
  asm volatile ("pop {r4-r11, pc}");
}
