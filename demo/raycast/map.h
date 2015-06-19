#ifndef DEMO_RAYCAST_MAP_H
#define DEMO_RAYCAST_MAP_H

#include <cstdint>

#include "demo/raycast/config.h"

namespace demo {
namespace raycast {

struct Map {
  std::uint8_t tiles[config::map_width * config::map_height];

  inline std::uint8_t fetch(unsigned x, unsigned y) const {
    return tiles[y * config::map_width + x];
  }
};

extern Map const canned_map;

}  // namespace raycast
}  // namespace demo

#endif  // DEMO_RAYCAST_MAP_H
