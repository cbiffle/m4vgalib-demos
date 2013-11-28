depth    := ../..

products := stm32f4xx

stm32f4xx[type] := library

stm32f4xx[cc_files] := \
  rcc.cc

stm32f4xx[cc_flags] := -std=gnu++0x

stm32f4xx[libs] := \
  lib/armv7m:armv7m \
  lib/biffield:biffield \
  lib/common:common

stm32f4xx[using_l_flags] := $(shell pwd)/peripherals.ld

include $(depth)/build/Makefile.rules
