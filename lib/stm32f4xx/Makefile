depth    := ../..

products := stm32f4xx

stm32f4xx[type] := library

stm32f4xx[cc_files] := \
  usart.cc

stm32f4xx[cc_flags] := -std=gnu++0x

stm32f4xx[libs] := \
  etl/armv7m:armv7m \
  etl/stm32f4xx:stm32f4xx \
  etl/biffield:biffield \
  lib/common:common

stm32f4xx[using_l_flags] := $(shell pwd)/peripherals.ld

include $(depth)/build/Makefile.rules
