#include "lib/armv7m/crt0.h"
#include "lib/armv7m/exception_table.h"
#include "lib/armv7m/instructions.h"
#include "lib/armv7m/scb.h"

#include "runtime/ramcode.h"

#include "vga/vga.h"
#include "vga/mode/text_800x600.h"

static vga::mode::Text_800x600 mode;

typedef vga::Mode::Pixel Pixel;

static void text_centered(unsigned row, Pixel fore, Pixel back, char const *s) {
  unsigned len = 0;
  while (s[len]) ++len;

  unsigned left_margin = 40 - len / 2;
  unsigned right_margin = 80 - len - left_margin;

  mode.cursor_to(0, row);
  for (unsigned i = 0; i < left_margin; ++i) mode.type_char(fore, back, ' ');
  mode.type_chars(fore, back, s);
  for (unsigned i = 0; i < right_margin; ++i) mode.type_char(fore, back, ' ');
}

static void text_at(unsigned col, unsigned row,
                    Pixel fore, Pixel back,
                    char const *s) {
  mode.cursor_to(col, row);
  mode.type_chars(fore, back, s);
}

enum {
  white   = 0b111111,
  lt_gray = 0b101010,
  dk_gray = 0b010101,
  black   = 0b000000,

  red     = 0b000011,
  blue    = 0b110000,
};

void v7m_reset_handler() {
  armv7m::crt0_init();
  runtime::ramcode_init();

  // Enable fault reporting.
  armv7m::scb.write_shcsr(armv7m::scb.read_shcsr()
                          .with_memfaultena(true)
                          .with_busfaultena(true)
                          .with_usgfaultena(true));

  // Enable floating point automatic/lazy state preservation.
  // The CONTROL bit governing FP will be set automatically when first used.
  armv7m::scb_fp.write_fpccr(armv7m::scb_fp.read_fpccr()
                             .with_aspen(true)
                             .with_lspen(true));
  armv7m::instruction_synchronization_barrier();  // Now please.

  // Enable access to the floating point coprocessor.
  armv7m::scb.write_cpacr(armv7m::scb.read_cpacr()
                          .with_cp11(armv7m::Scb::CpAccess::full)
                          .with_cp10(armv7m::Scb::CpAccess::full));

  // It is now safe to use floating point.

  vga::init();

  vga::select_mode(&mode);

  mode.clear_framebuffer(0);

  text_centered(0, white, dk_gray, "800x600 Attributed Text Demo");
  text_at(0, 1, white, black,
    "10x16 point characters in an 80x37 grid, with ");
  mode.type_chars(red, black, "foreground");
  mode.type_chars(white, black, " and ");
  mode.type_chars(white, blue, "background");
  mode.type_chars(white, black, " colors.");

  text_at(0, 3, black, white, "Colors chosen from 256-color palette"
                              " (only 64 currently wired up):\n");

  for (unsigned i = 0; i < 64; ++i) {
    mode.type_char(black, i, ' ');
  }
  mode.type_char(black, black, '\n');
  for (unsigned i = 0; i < 26; ++i) {
    mode.type_char(i, i ? black : dk_gray, 'A' + i);
  }
  for (unsigned i = 26; i < 64; ++i) {
    mode.type_char(i, black, 'A' + (i - 26));
  }

  text_at(0, 7, white, 0b100000,
    "     Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam ut\n"
    "     tellus quam. Cras ornare facilisis sollicitudin. Quisque quis\n"
    "     imperdiet mauris. Proin malesuada nibh dolor, eu luctus mauris\n"
    "     ultricies vitae. Interdum et malesuada fames ac ante ipsum primis\n"
    "     in faucibus. Aenean tincidunt viverra ultricies. Quisque rutrum\n"
    "     vehicula pulvinar.\n"
    "\n"
    "     Etiam commodo dui quis nibh dignissim laoreet. Aenean erat justo,\n"
    "     hendrerit ac adipiscing tempus, suscipit quis dui. Vestibulum ante\n"
    "     ipsum primis in faucibus orci luctus et ultrices posuere cubilia\n"
    "     Curae; Proin tempus bibendum ultricies. Etiam sit amet arcu quis\n"
    "     diam dictum suscipit eu nec odio. Donec cursus hendrerit porttitor.\n"
    "     Suspendisse ornare, diam vitae faucibus dictum, leo enim vestibulum\n"
    "     neque, id tempor tellus sem pretium lectus. Maecenas nunc nisl,\n"
    "     aliquam non quam at, vulputate lacinia dolor. Vestibulum nisi orci,\n"
    "     viverra ut neque semper, imperdiet laoreet ligula. Nullam venenatis\n"
    "     orci eget nibh egestas, sit amet sollicitudin erat cursus.\n"
    "\n"
    "     Nullam id ornare tellus, vel porta lectus. Suspendisse pretium leo\n"
    "     enim, vel elementum nibh feugiat non. Etiam non vulputate quam, sit\n"
    "     amet semper ante. In fermentum imperdiet sem non consectetur. Donec\n"
    "     egestas, massa a fermentum viverra, lectus augue hendrerit odio,\n"
    "     vitae molestie nibh nunc ut metus. Nulla commodo, lacus nec\n"
    "     interdum dignissim, libero dolor consequat mi, non euismod velit\n"
    "     sem nec dui. Praesent ligula turpis, auctor non purus eu,\n"
    "     adipiscing pellentesque felis.\n"
    );

  text_at(50, 36, white, black, "Frame number:");

  char fc[17];
  fc[16] = 0;
  unsigned frame = 0;
  while (1) {
    while (vga::in_vblank());
    vga::wait_for_vblank();
    ++frame;
    
    unsigned f = frame;
    for (unsigned i = 16; i > 0; --i) {
      unsigned n = f & 0xF;
      fc[i - 1] = n > 9 ? 'A' + n - 10 : '0' + n;
      f >>= 4;
    }
    text_at(64, 36, red, black, fc);
  }
}
