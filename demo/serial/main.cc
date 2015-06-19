#include "etl/scope_guard.h"

#include "etl/armv7m/implicit_crt0.h"

#include "etl/stm32f4xx/ahb.h"
#include "etl/stm32f4xx/apb.h"
#include "etl/stm32f4xx/gpio.h"
#include "etl/stm32f4xx/interrupts.h"
#include "etl/stm32f4xx/rcc.h"
#include "etl/stm32f4xx/usart.h"

#include "vga/arena.h"
#include "vga/rast/text_10x16.h"
#include "vga/timing.h"
#include "vga/vga.h"

#include "demo/config.h"
#include "demo/terminal.h"

DEMO_REQUIRE_RESOLUTION(800, 600)

using etl::stm32f4xx::AhbPeripheral;
using etl::stm32f4xx::ApbPeripheral;
using etl::stm32f4xx::gpioa;
using etl::stm32f4xx::rcc;
using etl::stm32f4xx::Usart;
using etl::stm32f4xx::usart2;
using etl::stm32f4xx::Gpio;

using Pixel = vga::Rasterizer::Pixel;

/*******************************************************************************
 * Some basic terminal functionality.
 */

struct TextDemo : public demo::Terminal {
  TextDemo() : demo::Terminal(800, 600) {
    rasterizer.clear_framebuffer(demo::blue);
  }

  vga::Band const band{&rasterizer, 600, nullptr};

  void type_decimal(Pixel fore, Pixel back, unsigned n,
                    bool right = false) {
    char buf[10] = { ' ' };
    unsigned p = 10;
    do {
      buf[--p] = '0' + (n % 10);
      n /= 10;
    } while (n);

    for (unsigned i = right ? 0 : p; i < 10; ++i) {
      type(fore, back, buf[i]);
    }
  }

  void type_box(Pixel fore, Pixel back,
                unsigned left, unsigned top,
                unsigned right, unsigned bottom) {
    cursor_to(left, top);
    type_raw(fore, back, '+');
    for (unsigned x = left + 1; x < right; ++x) {
      type_raw(fore, back, '-');
    }
    type_raw(fore, back, '+');

    for (unsigned y = top + 1; y < bottom; ++y) {
      cursor_to(left, y);
      type_raw(fore, back, '|');
      for (unsigned x = left + 1; x < right; ++x) {
        type_raw(fore, back, ' ');
      }
      type_raw(fore, back, '|');
    }

    cursor_to(left, bottom);
    type_raw(fore, back, '+');
    for (unsigned x = left + 1; x < right; ++x) {
      type_raw(fore, back, '-');
    }
    type_raw(fore, back, '+');
  }
};


/*******************************************************************************
 * The actual demo.
 */

static void usart2_init() {
  rcc.enable_clock(AhbPeripheral::gpioa);
  rcc.leave_reset(AhbPeripheral::gpioa);

  rcc.enable_clock(ApbPeripheral::usart2);
  rcc.leave_reset(ApbPeripheral::usart2);

  // Enable the USART before other actions.
  usart2.write_cr1(Usart::cr1_value_t().with_ue(true));

  float clock = rcc.get_clock_hz(ApbPeripheral::usart2);
  unsigned brr = static_cast<unsigned>(clock / 115200.f + 0.5f);
  usart2.write_brr(Usart::brr_value_t()
                   .with_div_mantissa(brr >> 4)
                   .with_div_fraction(brr & 0xF));

  // Turn on the transmitter.
  usart2.write_cr1(usart2.read_cr1()
                   .with_te(true)
                   .with_re(true));

  unsigned short pins = Gpio::p2 | Gpio::p3;
  gpioa.set_mode(pins, Gpio::Mode::alternate);
  gpioa.set_output_type(pins, Gpio::OutputType::push_pull);
  gpioa.set_output_speed(pins, Gpio::OutputSpeed::medium_25mhz);
  gpioa.set_pull(pins, Gpio::Pull::none);
  gpioa.set_alternate_function(pins, 7);  // USART2_TX/RX
}

template <unsigned N>
class Queue {
public:
  Queue() = default;

  bool offer(unsigned char c) {
    if (is_full()) return false;
    _bytes[_writer++ % N] = c;
    return true;
  }

  bool take(unsigned char &c) {
    if (is_empty()) return false;
    c = _bytes[_reader++ % N];
    return true;
  }

  bool is_full() const {
    return _writer == _reader + N;
  }

  bool is_empty() const {
    return _writer == _reader;
  }

private:
  unsigned char _bytes[N];
  unsigned volatile _reader;
  unsigned volatile _writer;
};

static Queue<16> usart_rx_queue;
static Queue<16> usart_tx_queue;

static void usart2_send(unsigned char b) {
  (void) usart_tx_queue.offer(b);
}

__attribute__((section(".ramcode")))
static void usart2_poll() {
  auto sr = usart2.read_sr();

  if (sr.get_rxne()) {
    usart_rx_queue.offer(usart2.read_dr().get_dr());
  }

  if (sr.get_txe()) {
    unsigned char c;
    if (usart_tx_queue.take(c)) {
      usart2.write_dr(c);
    }
  }
}

static void startup_banner(TextDemo & d) {
  using namespace demo;  // for colors

  d.type_box(white, dk_gray, 10, 10, 70, 27);

  d.cursor_to(11, 11);
  d.type(white, dk_gray, "PixelPusher v1 - arena: ");
  d.type_decimal(white, dk_gray, vga::arena_bytes_free());
  d.type(white, dk_gray, " / ");
  d.type_decimal(white, dk_gray, vga::arena_bytes_total());
  d.type(white, dk_gray, " bytes free.");

  d.cursor_to(11, 12); d.type(white, dk_gray, "Clocks:");

  d.cursor_to(14, 13);
  d.type(white, dk_gray, "CPU:   ");
  d.type_decimal(white, dk_gray,
                 static_cast<unsigned>(rcc.get_cpu_clock_hz()),
                 true);

  d.cursor_to(14, 14);
  d.type(white, dk_gray, "AHBx:  ");
  d.type_decimal(white, dk_gray,
                 static_cast<unsigned>(rcc.get_ahb_clock_hz()),
                 true);

  d.cursor_to(14, 15);
  d.type(white, dk_gray, "APB1:  ");
  d.type_decimal(white, dk_gray,
                 static_cast<unsigned>(rcc.get_apb1_clock_hz()),
                 true);

  d.cursor_to(14, 16);
  d.type(white, dk_gray, "APB2:  ");
  d.type_decimal(white, dk_gray,
                 static_cast<unsigned>(rcc.get_apb2_clock_hz()),
                 true);

  d.cursor_to(14, 17);
  d.type(white, dk_gray, "PLL48: ");
  d.type_decimal(white, dk_gray,
                 static_cast<unsigned>(rcc.get_pll48_clock_hz()),
                 true);

  d.cursor_to(14, 18);
  d.type(white, dk_gray, "Pixel: ");
  d.type_decimal(white, dk_gray, 
                 static_cast<unsigned>(rcc.get_cpu_clock_hz() / 4),
                 true);

  d.cursor_to(26, 26);
  d.type(white, dk_gray, "Press any key to continue");
}

int main() {
  vga::init();
  vga::configure_timing(vga::timing_vesa_800x600_60hz);

  auto d = vga::arena_make<TextDemo>();

  usart2_init();

  startup_banner(*d);

  vga::configure_band_list(&d->band);
  ETL_ON_SCOPE_EXIT { vga::clear_band_list(); };

  vga::video_on();

  // Wait for first character.
  while (true) {
    unsigned char c;
    bool has_c = usart_rx_queue.take(c);
    if (!has_c) continue;

    d->rasterizer.clear_framebuffer(demo::blue);
    d->cursor_to(0, 0);
    usart2_send(c);
    d->type(demo::white, demo::blue, c);
    break;
  }

  // Subsequent characters.
  while (true) {
    unsigned char c;
    bool has_c = usart_rx_queue.take(c);
    if (!has_c) continue;

    usart2_send(c);
    d->type(demo::white, demo::blue, c);
  }
  __builtin_unreachable();
}

__attribute__((section(".ramcode")))
void vga_hblank_interrupt() {
  usart2_poll();
}
