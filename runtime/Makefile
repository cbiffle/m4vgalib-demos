depth := ..
products := runtime default_traps

runtime[type] := library

runtime[cc_files] := runtime.cc startup.cc

runtime[cc_flags] := -std=gnu++0x

runtime[libs] := \
  etl/armv7m:armv7m \
  etl/stm32f4xx:stm32f4xx


default_traps[type] := library

default_traps[cc_files] := traps.cc

default_traps[cc_flags] := -std=gnu++0x

default_traps[libs] := \
  etl/armv7m:armv7m \

include $(depth)/build/Makefile.rules
