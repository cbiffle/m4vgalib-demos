#ifndef DEMO_WIPE_CONFIG_H
#define DEMO_WIPE_CONFIG_H

#include "demo/config.h"

DEMO_REQUIRE_RESOLUTION(800, 600)

namespace demo {
namespace wipe {
namespace config {

static constexpr int
  cols = 800,
  rows = 600;

static constexpr float
  pi = 3.1415926f;

static constexpr unsigned
  max_band_height = 3 * 16;

}  // namespace config
}  // namespace wipe
}  // namespace demo

#endif  // DEMO_WIPE_CONFIG_H
