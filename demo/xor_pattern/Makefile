depth := ../..
products := lib demo

lib[type] := library

lib[cc_files] := \
  mode_800x600.cc

lib[S_files] := \
  pattern.S

lib[cc_flags] := -std=gnu++0x

lib[libs] := \
  lib/stm32f4xx:stm32f4xx \
  vga:vga


demo[type] := program
demo[arch] := stm32f4xx

demo[cc_files] := main.cc

demo[cc_flags] := -std=gnu++0x -nostdinc

demo[libs] := \
  lib/armv7m:armv7m \
  lib/armv7m:crt0 \
  lib/armv7m:exception_table \
  lib/stm32f4xx:interrupt_table \
  runtime:runtime \
  runtime:default_traps \
  vga:vga \
  demo/xor_pattern:lib

demo[whole_archive] := \
  lib/armv7m:exception_table \
  lib/stm32f4xx:interrupt_table \
  runtime:default_traps

demo[l_flags] = \
  -Wl,-Map=$(intermediate)/demo.map \
  -T$(shell pwd)/$(depth)/m4vga-stm32f4xx.ld \
  -nostdlib -nodefaultlibs


include $(depth)/build/Makefile.rules


flash: $(intermediate)/demo
	openocd -f openocd.cfg \
                -c "init" \
                -c "reset init" \
                -c "flash write_image erase $<" \
                -c "reset halt" \
                -c "resume" \
                -c "shutdown"
