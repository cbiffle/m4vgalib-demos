#include "demo/runner.h"

#include "demo/conway/conway.h"
#include "demo/tunnel/tunnel.h"
#include "demo/hires_mix/hires_mix.h"
#include "demo/hires_text/hires_text.h"
#include "demo/rook/rook.h"
#include "demo/wipe/wipe.h"
#include "demo/xor_pattern/xor.h"

int main() {
  demo::general_setup();

  while (true) {
    demo::hires_text::legacy_run();
    demo::hires_mix::run();
    demo::xor_pattern::run();
    demo::wipe::legacy_run();
    demo::tunnel::run();
    demo::conway::legacy_run();
    demo::rook::legacy_run();
  }
}
