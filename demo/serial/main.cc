#include "etl/armv7m/exception_table.h"

#include "etl/stm32f4xx/ahb.h"
#include "etl/stm32f4xx/apb.h"
#include "etl/stm32f4xx/gpio.h"
#include "etl/stm32f4xx/interrupts.h"
#include "etl/stm32f4xx/rcc.h"
#include "etl/stm32f4xx/usart.h"

#include "etl/armv7m/crt0.h"

#include "vga/arena.h"
#include "vga/rast/text_10x16.h"
#include "vga/timing.h"
#include "vga/vga.h"

using etl::stm32f4xx::AhbPeripheral;
using etl::stm32f4xx::ApbPeripheral;
using etl::stm32f4xx::gpioa;
using etl::stm32f4xx::rcc;
using etl::stm32f4xx::Usart;
using etl::stm32f4xx::usart2;
using etl::stm32f4xx::Gpio;

static vga::rast::Text_10x16 rasterizer(800, 600);

static vga::Band const band = { &rasterizer, 600, nullptr };

typedef vga::Rasterizer::Pixel Pixel;

/*******************************************************************************
 * Some basic terminal functionality.
 */

static unsigned t_row = 0, t_col = 0;

static void type_raw(Pixel fore, Pixel back, char c) {
  rasterizer.put_char(t_col, t_row, fore, back, c);
  ++t_col;
  if (t_col == 80) {
    t_col = 0;
    ++t_row;
    if (t_row == 37) t_row = 0;
  }
}

static void cursor_to(unsigned col, unsigned row) {
  if (col >= 80) col = 80 - 1;
  if (row >= 37) row = 37 - 1;

  t_col = col;
  t_row = row;
}

static void type(Pixel fore, Pixel back, char c) {
  switch (c) {
    case '\r':
      do {
        type_raw(fore, back, ' ');
      } while (t_col);
      return;

    case '\f':
      rasterizer.clear_framebuffer(back);
      cursor_to(0, 0);
      break;

    case '\b':
      if (t_col) {
        --t_col;
        type_raw(fore, back, ' ');
        --t_col;
      }
      break;


    default:
      type_raw(fore, back, c);
      return;
  }
}

static void type(Pixel fore, Pixel back, char const *s) {
  while (char c = *s++) type(fore, back, c);
}

static void type_decimal(Pixel fore, Pixel back, unsigned n,
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

static void type_box(Pixel fore, Pixel back,
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

enum {
  white   = 0b111111,
  lt_gray = 0b101010,
  dk_gray = 0b010101,
  black   = 0b000000,

  red     = 0b000011,
  green   = 0b001100,
  blue    = 0b110000,
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

/*
 * Interrupts!
 */

void etl_armv7m_reset_handler() {
  etl::armv7m::crt0_init();
  vga::init();

  rasterizer.activate(vga::timing_vesa_800x600_60hz);
  vga::configure_band_list(&band);
  vga::configure_timing(vga::timing_vesa_800x600_60hz);

  usart2_init();

  rasterizer.clear_framebuffer(blue);

  type_box(white, dk_gray, 10, 10, 70, 27);

  cursor_to(11, 11);
  type(white, dk_gray, "PixelPusher v1 - arena: ");
  type_decimal(white, dk_gray, vga::arena_bytes_free());
  type(white, dk_gray, " / ");
  type_decimal(white, dk_gray, vga::arena_bytes_total());
  type(white, dk_gray, " bytes free.");

  cursor_to(11, 12); type(white, dk_gray, "Clocks:");

  cursor_to(14, 13);
  type(white, dk_gray, "CPU:   ");
  type_decimal(white, dk_gray,
               static_cast<unsigned>(rcc.get_cpu_clock_hz()),
               true);

  cursor_to(14, 14);
  type(white, dk_gray, "AHBx:  ");
  type_decimal(white, dk_gray,
               static_cast<unsigned>(rcc.get_ahb_clock_hz()),
               true);

  cursor_to(14, 15);
  type(white, dk_gray, "APB1:  ");
  type_decimal(white, dk_gray,
               static_cast<unsigned>(rcc.get_apb1_clock_hz()),
               true);

  cursor_to(14, 16);
  type(white, dk_gray, "APB2:  ");
  type_decimal(white, dk_gray,
               static_cast<unsigned>(rcc.get_apb2_clock_hz()),
               true);

  cursor_to(14, 17);
  type(white, dk_gray, "PLL48: ");
  type_decimal(white, dk_gray,
               static_cast<unsigned>(rcc.get_pll48_clock_hz()),
               true);

  cursor_to(14, 18);
  type(white, dk_gray, "Pixel: ");
  type_decimal(white, dk_gray, 
               static_cast<unsigned>(rcc.get_cpu_clock_hz() / 4),
               true);

  cursor_to(26, 26);
  type(white, dk_gray, "Press any key to continue");

  cursor_to(0, 0);

  vga::video_on();

  while (true) {
    unsigned char c;
    bool has_c = usart_rx_queue.take(c);
    if (!has_c) continue;

    rasterizer.clear_framebuffer(blue);
    usart2_send(c);
    type(white, blue, c);
    break;
  }

  while (true) {
    unsigned char c;
    bool has_c = usart_rx_queue.take(c);
    if (!has_c) continue;

    usart2_send(c);
    type(white, blue, c);
  }
}

__attribute__((section(".ramcode")))
void vga_hblank_interrupt() {
  usart2_poll();
}
