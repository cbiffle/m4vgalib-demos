/*
 * This provides specialized C runtime startup for our memory map.
 */

#include "runtime/startup.h"

#include "etl/common/attribute_macros.h"

#include "etl/armv7m/types.h"
#include "etl/armv7m/instructions.h"
#include "etl/armv7m/scb.h"
#include "etl/armv7m/scb_fp.h"

#include "etl/stm32f4xx/rcc.h"
#include "etl/stm32f4xx/syscfg.h"

namespace armv7m = etl::armv7m;
using etl::armv7m::Word;

using armv7m::scb;

using etl::stm32f4xx::ApbPeripheral;
using etl::stm32f4xx::rcc;
using etl::stm32f4xx::SysCfg;
using etl::stm32f4xx::syscfg;

typedef void (*InitFnPtr)();

/*
 * These symbols are provided by the linker script.
 */
extern "C" {
  // Start of image to be copied.
  extern Word _data_init_image_start;
  // Destination for copy (RAM).
  extern Word _data_start;
  // End of destination
  extern Word _data_end;

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

static void enable_floating_point() {
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
}

static void remap_sram() {
  // Remap SRAM.
  // Power on syscfg, so we can mess with its registers.
  rcc.enable_clock(ApbPeripheral::syscfg);

  // VTOR starts out at zero, which points to Flash and is good.
  // But now things are about to change.
  // Interrupts are disabled, but to be safe in case we fault (due to a bug),
  // go ahead and give VTOR the true address of the Flash table.
  scb.write_vtor(reinterpret_cast<unsigned>(&_data_start));
  armv7m::data_synchronization_barrier();  // Write it now.
  armv7m::instruction_synchronization_barrier();  // Flush pipeline just in case

  // Remap!
  auto mode = SysCfg::memrmp_value_t::mem_mode_t::embedded_sram;
  syscfg.write_memrmp(syscfg.read_memrmp()
                      .with_mem_mode(mode));
  armv7m::data_synchronization_barrier();  // Write it now.
  armv7m::instruction_synchronization_barrier();  // Flush pipeline just in case
}

void crt_init() {
  armv7m::scb.enable_faults();
  enable_floating_point();

  // Copy image into SRAM.  Note that this covers both initialized data and
  // RAM-resident code.
  {
    Word const *src = &_data_init_image_start;
    Word *dest = &_data_start;
    while (dest < &_data_end) {
      *dest++ = *src++;
    }
  }

  remap_sram();

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
