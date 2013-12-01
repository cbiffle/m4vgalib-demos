depth := .
products := m4vga stuff

m4vga[type] := program
m4vga[arch] := stm32f4xx

m4vga[cc_files] := main.cc runtime.cc

m4vga[cc_flags] := -std=gnu++0x

m4vga[libs] := \
  lib/armv7m:armv7m \
  lib/armv7m:crt0 \
  lib/armv7m:exception_table \
  lib/stm32f4xx:stm32f4xx \
  lib/stm32f4xx:interrupt_table \
  vga:vga

m4vga[whole_archive] := \
  lib/armv7m:exception_table \
  lib/stm32f4xx:interrupt_table

m4vga[l_flags] = \
  -Wl,-Map=$(intermediate)/m4vga.map \
  -T$(shell pwd)/m4vga-stm32f4xx.ld \
  -nostdlib -nodefaultlibs -nostdinc


stuff[type] := subdirectories


include $(depth)/build/Makefile.rules

flash: $(intermediate)/m4vga
	openocd -f openocd.cfg \
                -c "init" \
                -c "reset init" \
                -c "flash write_image erase $<" \
                -c "reset halt" \
                -c "resume" \
                -c "shutdown"
