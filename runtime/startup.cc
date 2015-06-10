#include "etl/attribute_macros.h"

#include "etl/armv7m/types.h"
#include "etl/armv7m/instructions.h"
#include "etl/armv7m/scb.h"
#include "etl/armv7m/scb_fp.h"

#include "etl/stm32f4xx/rcc.h"
#include "etl/stm32f4xx/syscfg.h"

using etl::armv7m::Word;

using etl::armv7m::scb;

using etl::stm32f4xx::ApbPeripheral;
using etl::stm32f4xx::rcc;
using etl::stm32f4xx::SysCfg;
using etl::stm32f4xx::syscfg;

extern "C" {
  extern etl::armv7m::Word _data_start;
}

/*
 * The type of things that can go into the preinit_array (void functions).
 */
typedef void (*InitFnPtr)();

/*
 * Turn on the CPU's fault reporting during preinit, e.g. before constructors
 * run.  Without this, memory violations, usage faults, etc. just show up as
 * Hard Fault and are a real pain to debug.
 */
static void enable_faults() {
  etl::armv7m::scb.enable_faults();
}

ETL_SECTION(".preinit_array") ETL_USED
InitFnPtr const preinit_enable_faults = enable_faults;

/*
 * Turn on the floating point unit for all execution contexts, with lazy
 * save enabled to make interrupts cheaper.
 *
 * We do this before running constructors so that everyone can just pretend
 * it's always on.  To do otherwise risks a Usage Fault.
 */
static void enable_floating_point() {
  // Enable floating point automatic/lazy state preservation.
  // The CONTROL bit governing FP will be set automatically when first used.
  etl::armv7m::scb_fp.write_fpccr(etl::armv7m::scb_fp.read_fpccr()
                                  .with_aspen(true)
                                  .with_lspen(true));

  // Enable access to the floating point coprocessor.
  etl::armv7m::scb.write_cpacr(etl::armv7m::scb.read_cpacr()
                               .with_cp11(etl::armv7m::Scb::CpAccess::full)
                               .with_cp10(etl::armv7m::Scb::CpAccess::full));
  etl::armv7m::instruction_synchronization_barrier();  // Now please.
}

ETL_SECTION(".preinit_array") ETL_USED
InitFnPtr const preinit_enable_floating_point = enable_floating_point;

/*
 * Remap SRAM112 to addresses starting at 0x00000000.  We do this before
 * running constructors to simplify the init process -- otherwise code would be
 * aware of its data being at address A one minute and address B the next.  In
 * particular, this lets us combine SRAM112 initialization with the rodata
 * initialization in the CRT startup.
 */
static void remap_sram() {
  // Power on syscfg, so we can mess with its registers.
  rcc.enable_clock(ApbPeripheral::syscfg);

  // Note: we do not need to change VTOR here.
  //
  // It is initially zero.  At boot, this aliases the ROM copy of the table.
  //
  // The table has been copied to RAM *before* the preinit_array functions run.
  // The remapping we're about to perform will cause address zero (and thus the
  // VTOR) to alias the *RAM* copy of the vector table.

  // Remap!
  auto mode = SysCfg::memrmp_value_t::mem_mode_t::embedded_sram;
  syscfg.write_memrmp(syscfg.read_memrmp()
                      .with_mem_mode(mode));
  etl::armv7m::data_synchronization_barrier();  // Write it now.
  etl::armv7m::instruction_synchronization_barrier();
}

ETL_SECTION(".preinit_array") ETL_USED
InitFnPtr const preinit_remap_sram = remap_sram;
