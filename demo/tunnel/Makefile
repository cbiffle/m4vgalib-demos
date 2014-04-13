depth := ../..
products := lib demo

lib[type] := library

lib[cc_files] := tunnel.cc

lib[cc_flags] := -std=gnu++0x -O2

lib[libs] := \
  etl/armv7m:armv7m \
  vga:vga

lib[using_l_flags] := -lm

demo[type] := program
demo[arch] := stm32f4xx

demo[cc_files] := main.cc

demo[cc_flags] := -std=gnu++0x -O2

demo[libs] := \
  etl/armv7m:exception_table \
  etl/stm32f4xx:interrupt_table \
  etl/armv7m:manual_crt0 \
  runtime:runtime \
  runtime:default_traps \
  vga:vga \
  demo/tunnel:lib

demo[whole_archive] := \
  etl/armv7m:exception_table \
  etl/stm32f4xx:interrupt_table \
  runtime:default_traps \
  runtime:runtime

demo[l_flags] = \
  -Wl,-Map=$(intermediate)/demo.map \
  -T$(shell pwd)/$(depth)/m4vga-stm32f4xx.ld \
  -nostartfiles


include $(depth)/build/Makefile.rules


flash: $(intermediate)/demo
	openocd -f $(depth)/openocd.cfg \
                -c "init" \
                -c "reset init" \
                -c "flash write_image erase $<" \
                -c "reset halt" \
                -c "resume" \
                -c "shutdown"
