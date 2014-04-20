depth := ..
products := reel

reel[type] := program
reel[arch] := stm32f4xx

reel[cc_files] := main.cc

reel[cc_flags] := -std=gnu++0x

reel[libs] := \
  etl/armv7m:armv7m \
  etl/armv7m:exception_table \
  etl/stm32f4xx:interrupt_table \
  etl/armv7m:manual_crt0 \
  runtime:runtime \
  runtime:default_traps \
  vga:vga \
  demo:demo \
  demo/tunnel:lib \
  demo/conway:lib \
  demo/hires_text:lib \
  demo/rook:lib

reel[whole_archive] := \
  etl/armv7m:exception_table \
  etl/stm32f4xx:interrupt_table \
  runtime:default_traps \
  runtime:runtime

reel[l_flags] = \
  -Wl,-Map=$(intermediate)/demo.map \
  -T$(shell pwd)/$(depth)/m4vga-stm32f4xx.ld \
  -nostartfiles


include $(depth)/build/Makefile.rules


flash: $(intermediate)/reel
	openocd -f $(depth)/openocd.cfg \
                -c "init" \
                -c "reset init" \
                -c "flash write_image erase $<" \
                -c "reset halt" \
                -c "resume" \
                -c "shutdown"
