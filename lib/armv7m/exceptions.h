#ifndef LIB_ARMV7M_EXCEPTIONS_H
#define LIB_ARMV7M_EXCEPTIONS_H

#include "lib/armv7m/types.h"

namespace armv7m {

/*
 * This provides number and name for each architectural exception, using the
 * same numbering as the IPSR and vectactive registers.
 */
enum class Exception {
  reset = 1,  // Not present in exceptions.def for convenience

  #define V7M_EXCEPTION(name) name,
  #define V7M_EXCEPTION_RESERVED(n) __reserved_ ## n,
  #include "lib/armv7m/exceptions.def"
  #undef V7M_EXCEPTION
  #undef V7M_EXCEPTION_RESERVED
};

/*
 * Sets the priority of a configurable-priority exception.  Note that SoC
 * vendors don't implement all 8 bits, so using this directly is dangerous.
 * The SoC driver layer should provide a version that shifts.
 */
void set_exception_priority(Exception, Byte);

}  // namespace armv7m

#endif  // LIB_ARMV7M_EXCEPTIONS_H
