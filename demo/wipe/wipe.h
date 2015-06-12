#ifndef DEMO_WIPE_H
#define DEMO_WIPE_H

#include "vga/vga.h"

#include "demo/terminal.h"
#include "demo/scene.h"
#include "demo/xor_pattern/rasterizer.h"
#include "demo/wipe/config.h"

namespace demo {
namespace wipe {

/*
 * Combines two rasterizers by overlaying one and sliding the overlay area
 * up and down, like a 'wipe' transition in film.
 */
class Wipe : public Scene {
public:
  void configure_band_list() override;
  bool render_frame(unsigned) override;

private:
  demo::xor_pattern::Rasterizer _border{config::cols};
  demo::Terminal _term{config::cols + 10,
                       config::max_band_height,
                       config::rows/2 - config::max_band_height/2};
  vga::Band _bands[3] {
    { &_border,          config::rows/2, &_bands[1] },
    { &_term.rasterizer, 0,              &_bands[2] },
    { &_border,          config::rows/2, nullptr },
  };
};

void legacy_run();

}  // namespace wipe
}  // namespace demo

#endif  // DEMO_WIPE_H
