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

typedef void (*InitFnPtr)();

static void enable_faults() {
  etl::armv7m::scb.enable_faults();
}

ETL_SECTION(".preinit_array") ETL_USED
InitFnPtr const preinit_enable_faults = enable_faults;

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

static void remap_sram() {
  // Remap SRAM.
  // Power on syscfg, so we can mess with its registers.
  rcc.enable_clock(ApbPeripheral::syscfg);

  // VTOR starts out at zero, which points to Flash and is good.
  // But now things are about to change.
  // Interrupts are disabled, but to be safe in case we fault (due to a bug),
  // go ahead and give VTOR the true address of the Flash table.
  // TODO(cbiffle): this is a no-op, remove it.
  scb.write_vtor(reinterpret_cast<unsigned>(&_data_start));
  etl::armv7m::data_synchronization_barrier();  // Write it now.
  etl::armv7m::instruction_synchronization_barrier();

  // Remap!
  auto mode = SysCfg::memrmp_value_t::mem_mode_t::embedded_sram;
  syscfg.write_memrmp(syscfg.read_memrmp()
                      .with_mem_mode(mode));
  etl::armv7m::data_synchronization_barrier();  // Write it now.
  etl::armv7m::instruction_synchronization_barrier();
}

ETL_SECTION(".preinit_array") ETL_USED
InitFnPtr const preinit_remap_sram = remap_sram;
