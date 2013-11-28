#ifndef LIB_ARMV7M_SCB_H
#define LIB_ARMV7M_SCB_H

namespace armv7m {

struct Scb {
  #define BFF_DEFINITION_FILE lib/armv7m/scb.reg
  #include <biffield/generate.h>
  #undef BFF_DEFINITION_FILE
};

extern Scb scb;

}  // namespace armv7m

#endif  // LIB_ARMV7M_SCB_H
