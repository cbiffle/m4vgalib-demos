#include "lib/armv7m/exception_table.h"

/*
 * We don't use any of these vectors, nor expect the associated exceptions
 * to occur, so trap them here.
 */

#define TRAP(name) \
  void v7m_ ## name ## _handler() { \
    while (1); \
  }

TRAP(nmi)
TRAP(hard_fault)
TRAP(mem_manage_fault)
TRAP(bus_fault)
TRAP(usage_fault)
TRAP(sv_call)
TRAP(debug_monitor)
TRAP(sys_tick)
