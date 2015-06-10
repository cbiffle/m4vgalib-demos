#ifndef DEMO_HIRES_TEXT_HIRES_TEXT_H
#define DEMO_HIRES_TEXT_HIRES_TEXT_H

#include "vga/vga.h"

#include "demo/scene.h"
#include "demo/terminal.h"

namespace demo {
namespace hires_text {

/*
 * A Scene that shows some static attributed text including a frame counter.
 * The frame counter mostly just serves to show the frame rate, and dates back
 * to the days when text rendering was expensive.
 */
class HiresText : public Scene {
public:
  static constexpr unsigned
    cols = 800,
    rows = 600;

  HiresText();

  void configure_band_list() override;
  bool render_frame(unsigned) override;

private:
  Terminal t{cols, rows};
  vga::Band const band { &t.rasterizer, rows, nullptr };
};

void legacy_run();

}  // namespace hires_text
}  // namespace demo

#endif  // DEMO_HIRES_TEXT_HIRES_TEXT_H
