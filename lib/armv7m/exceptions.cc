#include "lib/armv7m/exceptions.h"
#include "lib/armv7m/scb.h"
#include "lib/armv7m/types.h"

namespace armv7m {

void set_exception_priority(Exception e, Byte priority) {
  unsigned idx = static_cast<unsigned>(e);
  if (idx < 4 || idx > 15) {
    // Not configurable.
    // TODO(cbiffle): assert
    return;
  }

  unsigned reg = (idx - 4) / 4;
  unsigned slot = idx % 4;

  switch (reg) {
    case 0: scb.write_shpr1(scb.read_shpr1().with_pri(slot, priority)); break;
    case 1: scb.write_shpr2(scb.read_shpr2().with_pri(slot, priority)); break;
    case 2: scb.write_shpr3(scb.read_shpr3().with_pri(slot, priority)); break;
  }
}

}  // namespace armv7m
