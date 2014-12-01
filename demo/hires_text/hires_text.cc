#include "demo/hires_text/hires_text.h"

#include "etl/scope_guard.h"

#include "vga/arena.h"
#include "vga/rast/text_10x16.h"
#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/input.h"
#include "demo/terminal.h"

namespace demo {
namespace hires_text {

using Pixel = vga::Rasterizer::Pixel;

/*******************************************************************************
 * Some basic terminal functionality.
 */

struct TextDemo : public demo::Terminal {
  vga::Band const band{&rasterizer, 600, nullptr};

  TextDemo() : demo::Terminal(800, 600) {}

  void startup_banner() {
    text_centered(0, white, dk_gray, "800x600 Attributed Text Demo");
    text_at(0, 1, white, black,
        "10x16 point characters in an 80x37 grid, with ");
    type(red, black, "foreground");
    type(white, black, " and ");
    type(white, blue, "background");
    type(white, black, " colors.");

    text_centered(3, black, white,
        "Colors chosen from 256-color palette"
        " (only 64 currently wired up):");

    rainbow_type(
        "The quick brown fox jumped over the lazy dog. 0123456789!@#$%^&*");

    text_at(0, 7, white, 0b100000,
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

    text_at(3, 34, green, black, "Loadable fonts");
    type(white, black, " - ");
    type(blue, black, "Room for 14 text pages in RAM");
    type(white, black, " - ");
    type(red, black, "45.6% CPU cycles available");

    text_at(0, 36, white, black, "60 fps / 40MHz pixel clock");
    text_at(58, 36, white, black, "Frame number:");
  }
};


/*******************************************************************************
 * The actual demo.
 */

void run() {
  vga::arena_reset();
  input_init();

  auto d = vga::arena_make<TextDemo>();

  d->startup_banner();

  vga::configure_band_list(&d->band);
  ETL_ON_SCOPE_EXIT { vga::clear_band_list(); };

  vga::sync_to_vblank();
  vga::video_on();

  ETL_ON_SCOPE_EXIT {
    vga::sync_to_vblank();
    vga::video_off();
  };

  char fc[9];
  fc[8] = 0;
  unsigned frame = 0;
  while (!user_button_pressed()) {
    while (vga::in_vblank());
    vga::wait_for_vblank();
    
    unsigned f = ++frame;
    // Write out frame number as hex.
    for (unsigned i = 8; i > 0; --i) {
      unsigned n = f & 0xF;
      fc[i - 1] = n > 9 ? 'A' + n - 10 : '0' + n;
      f >>= 4;
    }
    d->text_at(72, 36, red, black, fc);
  }
}

}  // namespace hires_text
}  // namespace demo
