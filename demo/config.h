#ifndef DEMO_CONFIG_H
#define DEMO_CONFIG_H

#include "vga/timing.h"

namespace demo {
namespace config {

#if !defined(CFG_WIDTH) || !defined(CFG_HEIGHT) || !defined(CFG_HZ)
#  error CFG_WIDTH, CFG_HEIGHT, and CFG_HZ must be defined to use this.
#endif

static constexpr unsigned
  display_width = (CFG_WIDTH),
  display_height = (CFG_HEIGHT),
  notional_frame_rate = (CFG_HZ);

#define DEMO_MODE_(w, h, r) \
    vga::timing_vesa_ ## w ## x ## h ## _ ## r ## hz;
#define DEMO_MODE(w, h, r) DEMO_MODE_(w, h, r)

static vga::Timing const & timing = DEMO_MODE(CFG_WIDTH, CFG_HEIGHT, CFG_HZ);

#define DEMO_REQUIRE_RESOLUTION(w, h) \
  static_assert(::demo::config::display_width == w, \
      "This demo requires a display width of " #w); \
  static_assert(::demo::config::display_height == h, \
      "This demo requires a display height of " #h);

}  // namespace config
}  // namespace demo

#endif  // DEMO_CONFIG_H
