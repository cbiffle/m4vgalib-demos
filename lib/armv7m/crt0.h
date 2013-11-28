#ifndef LIB_ARMV7M_CRT0_H
#define LIB_ARMV7M_CRT0_H

namespace armv7m {

/*
 * Sets up basic invariants expected by C++ programs.  Normally called by the
 * reset handler, early on.
 */
void crt0_init();

}  // namespace armv7m

#endif  // LIB_ARMV7M_CRT0_H
