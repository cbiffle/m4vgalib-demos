#include "demo/runner.h"

#include "etl/scope_guard.h"

#include "vga/measurement.h"
#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/input.h"

namespace demo {

void general_setup() {
  vga::init();
  vga::msigs_init();
  input_init();

  vga::configure_timing(vga::timing_vesa_800x600_60hz);
}

bool start_button_pressed() {
  return user_button_pressed();
}

void loop_setup() {
  // Nothing to see here.
}

void run_scene(Scene & scene) {
  scene.configure_band_list();
  ETL_ON_SCOPE_EXIT { vga::clear_band_list(); };

  unsigned frame = 0;
  bool continue_scene;
  do {
    vga::msig_e_set(0);
    continue_scene = scene.render_frame(frame);
    vga::msig_e_clear(0);
    ++frame;
    
    vga::sync_to_vblank();
    vga::video_on();
  } while (continue_scene);
}

}  // namespace demo
