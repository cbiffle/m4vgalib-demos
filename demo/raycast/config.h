#ifndef DEMO_RAYCAST_CONFIG_H
#define DEMO_RAYCAST_CONFIG_H

namespace demo {
namespace raycast {
namespace config {

static constexpr unsigned
  disp_cols = 800,
  disp_rows = 600,
  div_x = 4,
  div_y = 1,
  map_width = 24,
  map_height = 24,
  tex_width = 64,
  tex_height = 32,
  apparent_tex_height = tex_height * 2;

static constexpr int
  cols = int(disp_cols) / div_x,
  rows = int(disp_rows) / div_y;

static constexpr float
  pi = 3.14159265358f,
  fov = 0.66f;

}  // namespace config
}  // namespace raycast
}  // namespace demo

#endif  // DEMO_RAYCAST_CONFIG_H
