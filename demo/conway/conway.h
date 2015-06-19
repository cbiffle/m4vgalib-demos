#ifndef DEMO_CONWAY_CONWAY_H
#define DEMO_CONWAY_CONWAY_H

#include "vga/vga.h"
#include "vga/rast/bitmap_1.h"

#include "demo/config.h"
#include "demo/scene.h"

DEMO_REQUIRE_RESOLUTION(800, 600)

namespace demo {
namespace conway {

/*
 * A Scene that runs Conway's Game of Life automaton, full screen, as fast as
 * I've been able to make it go.
 */
class Conway : public Scene {
public:
  static constexpr unsigned
    cols = 800,
    rows = 600;

  Conway();

  void configure_band_list() override;
  bool render_frame(unsigned) override;

private:
  vga::rast::Bitmap_1 rasterizer;
  vga::Band const bands[1] {
    { &rasterizer, rows, nullptr },
  };

  void set_random_cells(vga::Graphics1 &);
};

void legacy_run();

}  // namespace conway
}  // namespace demo

#endif  // DEMO_CONWAY_CONWAY_H
