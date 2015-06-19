#ifndef DEMO_RAYCAST_RAYCAST_H
#define DEMO_RAYCAST_RAYCAST_H

#include "math/geometry.h"

#include "vga/vga.h"
#include "vga/rast/palette8.h"
#include "vga/rast/palette8_mirror.h"

#include "demo/scene.h"
#include "demo/raycast/config.h"
#include "demo/raycast/hit.h"

namespace demo {
namespace raycast {

class RayCast : public Scene {
public:
  RayCast();

  void configure_band_list() override;
  bool render_frame(unsigned) override;
  
private:
  vga::rast::Palette8 _rasterizer{
    config::disp_cols, config::disp_rows / 2,
    config::div_x, config::div_y,
  };

  vga::rast::Palette8Mirror _mirror{
    _rasterizer,
    config::disp_rows / 2,
  };

  vga::Band const _bands[2] {
    { &_rasterizer, config::disp_rows / 2, &_bands[1] },
    { &_mirror,     config::disp_rows / 2, nullptr },
  };

  math::Vec2f _pos;     // Position of camera within map.
  math::Vec2f _dir;     // Direction vector of camera (unit).
  math::Vec2f _plane;   // Plane vector; perp. to _dir, length determines FOV.

  void update_camera();
  void rotate(float a);
  void move(math::Vec2f);

  Hit cast(float x) const;
};

}  // namespace raycast
}  // namespace demo

#endif  // DEMO_RAYCAST_TUMBLE_H
