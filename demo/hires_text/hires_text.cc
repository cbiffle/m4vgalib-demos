#include "demo/hires_text/hires_text.h"

#include "vga/arena.h"
#include "vga/rasterizer.h"

#include "demo/input.h"
#include "demo/runner.h"

namespace demo {
namespace hires_text {

using Pixel = vga::Rasterizer::Pixel;

/*******************************************************************************
 * Some basic terminal functionality.
 */

HiresText::HiresText() {
  t.text_centered(0, white, dk_gray, "800x600 Attributed Text Demo");
  t.text_at(0, 1, white, black,
      "10x16 point characters in an 80x37 grid, with ");
  t.type(red, black, "foreground");
  t.type(white, black, " and ");
  t.type(white, blue, "background");
  t.type(white, black, " colors.");

  t.text_centered(3, black, white,
      "Colors chosen from 256-color palette"
      " (only 64 currently wired up):");

  t.rainbow_type(
      "The quick brown fox jumped over the lazy dog. 0123456789!@#$%^&*");

  t.text_at(0, 7, white, 0b100000,
    "     Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam ut\n"
    "     tellus quam. Cras ornare facilisis sollicitudin. Quisque quis\n"
    "     imperdiet mauris. Proin malesuada nibh dolor, eu luctus mauris\n"
    "     ultricies vitae. Interdum et malesuada fames ac ante ipsum primis\n"
    "     in faucibus. Aenean tincidunt viverra ultricies. Quisque rutrum\n"
    "     vehicula pulvinar.\n"
    "\n"
    "     Etiam commodo dui quis nibh dignissim laoreet. Aenean erat justo,\n"
    "     hendrerit ac adipiscing tempus, suscipit quis dui. Vestibulum "
      "ante\n"
    "     ipsum primis in faucibus orci luctus et ultrices posuere cubilia\n"
    "     Curae; Proin tempus bibendum ultricies. Etiam sit amet arcu quis\n"
    "     diam dictum suscipit eu nec odio. Donec cursus hendrerit "
      "porttitor.\n"
    "     Suspendisse ornare, diam vitae faucibus dictum, leo enim "
      "vestibulum\n"
    "     neque, id tempor tellus sem pretium lectus. Maecenas nunc nisl,\n"
    "     aliquam non quam at, vulputate lacinia dolor. Vestibulum nisi "
      "orci,\n"
    "     viverra ut neque semper, imperdiet laoreet ligula. Nullam "
      "venenatis\n"
    "     orci eget nibh egestas, sit amet sollicitudin erat cursus.\n"
    "\n"
    "     Nullam id ornare tellus, vel porta lectus. Suspendisse pretium "
      "leo\n"
    "     enim, vel elementum nibh feugiat non. Etiam non vulputate quam, "
      "sit\n"
    "     amet semper ante. In fermentum imperdiet sem non consectetur. "
      "Donec\n"
    "     egestas, massa a fermentum viverra, lectus augue hendrerit odio,\n"
    "     vitae molestie nibh nunc ut metus. Nulla commodo, lacus nec\n"
    "     interdum dignissim, libero dolor consequat mi, non euismod velit\n"
    "     sem nec dui. Praesent ligula turpis, auctor non purus eu,\n"
    "     adipiscing pellentesque felis.\n"
    );

  t.text_at(3, 34, green, black, "Loadable fonts");
  t.type(white, black, " - ");
  t.type(blue, black, "Room for 14 text pages in RAM");
  t.type(white, black, " - ");
  t.type(red, black, "45.6% CPU cycles available");

  t.text_at(0, 36, white, black, "60 fps / 40MHz pixel clock");
  t.text_at(58, 36, white, black, "Frame number:");
}

void HiresText::configure_band_list() {
  vga::configure_band_list(&band);
}

bool HiresText::render_frame(unsigned frame) {
  bool continuing = !user_button_pressed();
  char fc[9];
  fc[8] = 0;
  // Write out frame number as hex.
  for (unsigned i = 8; i > 0; --i) {
    unsigned n = frame & 0xF;
    fc[i - 1] = n > 9 ? 'A' + n - 10 : '0' + n;
    frame >>= 4;
  }
  t.text_at(72, 36, red, black, fc);
  return continuing;
}


/*******************************************************************************
 * The actual demo.
 */

void legacy_run() {
  vga::arena_reset();
  auto scene = vga::arena_make<HiresText>();
  run_scene(*scene);
}

}  // namespace hires_text
}  // namespace demo
