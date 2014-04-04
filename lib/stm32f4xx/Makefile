depth    := ../..

products := stm32f4xx interrupt_table

stm32f4xx[type] := library

stm32f4xx[cc_files] := \
  gpio.cc \
  interrupts.cc \
  rcc.cc \
  usart.cc

stm32f4xx[cc_flags] := -std=gnu++0x

stm32f4xx[libs] := \
  etl/armv7m:armv7m \
  lib/biffield:biffield \
  lib/common:common

stm32f4xx[using_l_flags] := $(shell pwd)/peripherals.ld


interrupt_table[type] := library

interrupt_table[cc_files] := \
  interrupt_table.cc

interrupt_table[cc_flags] := -std=gnu++0x

interrupt_table[libs] := \
  lib/common:common

include $(depth)/build/Makefile.rules
