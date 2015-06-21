#ifndef DEMO_RAYCAST_HIT_H
#define DEMO_RAYCAST_HIT_H

#include "demo/raycast/config.h"

namespace demo {
namespace raycast {

struct Hit {
  enum class Side { x, y };

  unsigned texture;
  unsigned tex_u;
  float distance;
  Side side;
};

}  // namespace raycast
}  // namespace demo

#endif  // DEMO_RAYCAST_HIT_H
