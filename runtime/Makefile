depth := ..
products := runtime default_traps crt

runtime[type] := library

runtime[cc_files] := runtime.cc startup.cc

runtime[cc_flags] := -std=gnu++0x

runtime[libs] := \
  etl/armv7m:armv7m \
  etl/stm32f4xx:stm32f4xx


crt[type] := library

crt[cc_files] := crt.cc

crt[cc_flags] := -std=gnu++0x

crt[libs] := \
  etl/common:common \
  etl/armv7m:armv7m



default_traps[type] := library

default_traps[cc_files] := traps.cc

default_traps[cc_flags] := -std=gnu++0x

default_traps[libs] := \
  etl/armv7m:armv7m \

include $(depth)/build/Makefile.rules
