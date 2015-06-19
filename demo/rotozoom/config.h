#ifndef DEMO_ROTOZOOM_CONFIG_H
#define DEMO_ROTOZOOM_CONFIG_H

#include "demo/config.h"

DEMO_REQUIRE_RESOLUTION(800, 600)

namespace demo {
namespace rotozoom {
namespace config {

static constexpr int
  cols = 200,
  rows = 150;

static constexpr float
  pi = 3.1415926f;

}  // namespace config
}  // namespace rotozoom
}  // namespace demo

#endif  // DEMO_ROTOZOOM_CONFIG_H
