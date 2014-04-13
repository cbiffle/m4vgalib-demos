/*
 * This provides specialized C runtime startup for our memory map.
 */

#include "runtime/crt.h"

#include "etl/common/attribute_macros.h"

#include "etl/armv7m/types.h"

using etl::armv7m::Word;

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

void crt_init() {
  // Copy image into SRAM.  Note that this covers both initialized data and
  // RAM-resident code.
  {
    Word const *src = &_data_init_image_start;
    Word *dest = &_data_start;
    while (dest < &_data_end) {
      *dest++ = *src++;
    }
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
