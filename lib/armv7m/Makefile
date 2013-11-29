depth    := ../..

products := armv7m exception_table crt0

armv7m[type] := library

armv7m[cc_files] := \
  exceptions.cc \
  nvic.cc

armv7m[cc_flags] := -std=gnu++0x

armv7m[S_files] := \
  copy_words.S

armv7m[S_flags] := -Wa,-mimplicit-it=thumb

armv7m[libs] := \
  lib/biffield:biffield \
  lib/common:common

armv7m[using_l_flags] := $(shell pwd)/peripherals.ld


exception_table[type] := library

exception_table[cc_files] := \
  exception_table.cc

exception_table[cc_flags] := -std=gnu++0x

exception_table[libs] := \
  lib/common:common

exception_table[using_l_flags] := $(shell pwd)/peripherals.ld


crt0[type] := library
crt0[cc_files] := crt0.cc
crt0[cc_flags] := -std=gnu++0x
crt0[libs] := lib/common:common

include $(depth)/build/Makefile.rules
