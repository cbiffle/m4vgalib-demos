#ifndef DEMO_RAYCAST_TEXTURE_H
#define DEMO_RAYCAST_TEXTURE_H

#include "demo/raycast/config.h"

namespace demo {
namespace raycast {

struct Texture {
  std::uint8_t indices[config::tex_width * config::tex_height];

  inline std::uint8_t fetch(unsigned x, unsigned y) const {
    return indices[x * config::tex_height + y];
  }
};

}  // namespace raycast
}  // namespace demo

#endif  // DEMO_RAYCAST_TEXTURE_H
