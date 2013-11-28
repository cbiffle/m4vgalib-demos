#ifndef LIB_ARMV7M_EXCEPTION_TABLE_H
#define LIB_ARMV7M_EXCEPTION_TABLE_H

#include "lib/armv7m/types.h"

/*
 * ARMv7M Exceptions
 *
 * The architecture defines 16 exceptions, some of which are reserved.  These
 * signal processor-internal conditions, as opposed to interrupts, which are
 * defined by the SoC and signal external conditions.
 *
 * Exception names and ordering are defined in the exceptions.def x-macro
 * file.
 *
 * For ease of interfacing with assembly, exception handlers are defined as
 * C-style unmangled symbols in the top-level namespace.
 *
 * We don't provide any default exception linkage.  The application is
 * responsible for mapping *all* exceptions to handler functions, which may be
 * trivial.  (Note that this is explicitly different from CMSIS implementations,
 * which usually provide weak definitions.)
 */

/*
 * Handler hooks are named `v7m_foo_handler`, where `foo` is the name of the
 * exception as defined in exceptions.def.  e.g. v7m_reset_handler.  They're
 * defined with C linkage to allow them to be easily defined outside of this
 * namespace.
 */

extern "C" {
  void v7m_reset_handler();

  #define V7M_EXCEPTION(name) void v7m_ ## name ## _handler();
  #define V7M_EXCEPTION_RESERVED(n) /* nothing */
  #include "lib/armv7m/exceptions.def"
  #undef V7M_EXCEPTION
  #undef V7M_EXCEPTION_RESERVED

  /*
   * The exception table also contains the initial stack pointer, to be used
   * when calling the reset handler.  The application must define this, either
   * in code or the linker script, to be the word just above the initial stack.
   *
   * It's const because, typically, it's an address just above the top of RAM --
   * so you'd best not try to write it.
   */

  extern armv7m::Word const v7m_initial_stack_top;
}

#endif  // LIB_ARMV7M_EXCEPTION_TABLE_H
