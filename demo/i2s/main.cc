#include <math.h>

#include "etl/data/range_ptr.h"

#include "etl/armv7m/exception_table.h"

#include "etl/stm32f4xx/apb.h"
#include "etl/stm32f4xx/gpio.h"
#include "etl/stm32f4xx/interrupts.h"
#include "etl/stm32f4xx/rcc.h"
#include "etl/stm32f4xx/spi.h"
#include "etl/stm32f4xx/iic.h"

#include "runtime/crt.h"

#include "vga/timing.h"
#include "vga/vga.h"

using etl::data::RangePtr;

using etl::stm32f4xx::AhbPeripheral;
using etl::stm32f4xx::ApbPeripheral;
using etl::stm32f4xx::gpioa;
using etl::stm32f4xx::gpiob;
using etl::stm32f4xx::gpioc;
using etl::stm32f4xx::gpiod;
using etl::stm32f4xx::Rcc;
using etl::stm32f4xx::rcc;
using etl::stm32f4xx::Interrupt;
using etl::stm32f4xx::Iic;
using etl::stm32f4xx::iic1;
using etl::stm32f4xx::Gpio;
using etl::stm32f4xx::Spi;
using etl::stm32f4xx::spi3;

static void iic1_init() {
  // Turn on the IIC unit.
  rcc.enable_clock(ApbPeripheral::i2c1);

  float clock_hz = rcc.get_clock_hz(ApbPeripheral::i2c1);
  iic1.write_cr2(Iic::cr2_value_t()
                 .with_freq(clock_hz / 1e6f));
  iic1.write_ccr(Iic::ccr_value_t()
                 .with_fs(false)
                 .with_ccr(clock_hz / (2 * 100e3f)));

  iic1.write_trise(Iic::trise_value_t()
                   .with_trise(static_cast<unsigned>(clock_hz / 1e6f) + 1));

  iic1.write_cr1(Iic::cr1_value_t().with_pe(true));

  // We'll be using GPIOB for IIC.
  rcc.enable_clock(AhbPeripheral::gpiob);

  unsigned short const iic_pins = Gpio::p6 | Gpio::p9;
  gpiob.set_output_type(iic_pins, Gpio::OutputType::open_drain);
  gpiob.set_output_speed(iic_pins, Gpio::OutputSpeed::medium_25mhz);
  gpiob.set_pull(iic_pins, Gpio::Pull::none);
  gpiob.set_alternate_function(iic_pins, 4);
  gpiob.set_mode(iic_pins, Gpio::Mode::alternate);
}

static bool iic1_w(unsigned address, RangePtr<unsigned char> out) {
  // Set START
  iic1.write_cr1(iic1.read_cr1()
                 .with_start(true));
  // Wait for SB
  while (iic1.read_sr1().get_sb() == false);
  // Write address
  iic1.write_dr((address << 1));
  // Wait for ADDR, read SR1 and SR2
  while (true) {
    auto sr1 = iic1.read_sr1();
    if (sr1.get_addr()) {
      iic1.read_sr2();
      break;
    }

    if (sr1.get_af()) {
      return false;
    }
  }

  while (!out.is_empty()) {
    while (iic1.read_sr1().get_txe() == false);
    iic1.write_dr(out[0]);
    out = out.tail();
  }
  while (iic1.read_sr1().get_txe() == false);
  while (iic1.read_sr1().get_btf() == false);

  iic1.write_cr1(iic1.read_cr1().with_stop(true));
  while (iic1.read_cr1().get_stop() == true);

  return true;
}

static bool cs_write(unsigned char reg, unsigned char value) {
  unsigned char msg[] = { reg, value };
  return iic1_w(0x4A, msg);
}

static void i2s_init() {
  // Use PLLI2S for clock source.
  rcc.write_cfgr(rcc.read_cfgr()
                 .with_i2ssrc(Rcc::cfgr_value_t::i2ssrc_t::plli2s));

  // PLL input clock is 1MHz.
  // xN = 258MHz VCO frequency
  // /R = 86 MHz peripheral input clock
  rcc.write_plli2scfgr(Rcc::plli2scfgr_value_t()
                       .with_plli2sn(258)
                       .with_plli2sr(3));
  rcc.write_cr(rcc.read_cr().with_plli2son(true));
  while (rcc.read_cr().get_plli2srdy() == false);

  float i2sclk = 86e6f;

  rcc.enable_clock(AhbPeripheral::gpioa);
  rcc.enable_clock(AhbPeripheral::gpioc);
  rcc.enable_clock(ApbPeripheral::spi3_i2s3);

  spi3.write_i2scfgr(Spi::i2scfgr_value_t()
                     .with_i2smod(true)
                     .with_i2scfg(Spi::i2scfgr_value_t::i2scfg_t::master_tx)
                     .with_i2sstd(Spi::i2scfgr_value_t::i2sstd_t::i2s)
                     .with_ckpol(false)
                     .with_datlen(Spi::i2scfgr_value_t::datlen_t::bits_16)
                     .with_chlen(Spi::i2scfgr_value_t::chlen_t::bits_16));

  unsigned hz = static_cast<unsigned>((i2sclk / 256) / 48000 + 0.5f);

  spi3.write_i2spr(Spi::i2spr_value_t()
                   .with_i2sdiv(hz / 2)
                   .with_odd(hz & 1)
                   .with_mckoe(true));

  spi3.write_i2scfgr(spi3.read_i2scfgr().with_i2se(true));

  // Do SCLK, SD, MCLK
  unsigned short const i2s_pins = Gpio::p7 | Gpio::p10 | Gpio::p12;

  gpioc.set_output_type(i2s_pins, Gpio::OutputType::push_pull);
  gpioc.set_output_speed(i2s_pins, Gpio::OutputSpeed::fast_50mhz);
  gpioc.set_pull(i2s_pins, Gpio::Pull::none);
  gpioc.set_alternate_function(i2s_pins, 6);
  gpioc.set_mode(i2s_pins, Gpio::Mode::alternate);

  // Now WS, which lives on a different port.

  gpioa.set_output_type(Gpio::p4, Gpio::OutputType::push_pull);
  gpioa.set_output_speed(Gpio::p4, Gpio::OutputSpeed::fast_50mhz);
  gpioa.set_pull(Gpio::p4, Gpio::Pull::none);
  gpioa.set_alternate_function(Gpio::p4, 6);
  gpioa.set_mode(Gpio::p4, Gpio::Mode::alternate);
}

static bool cs_init() {

  (void) cs_write(0x04, 0b10101111);  // headphones on, speakers off
  (void) cs_write(0x0D, 0x70);
  (void) cs_write(0x05, 0x81);        // clock auto, div2
  (void) cs_write(0x06, 0b00000111);  // 16bit words, I2S protocol
  (void) cs_write(0x0A, 0);
  (void) cs_write(0x27, 0);
  (void) cs_write(0x1A, 0x0A);        // Volume left
  (void) cs_write(0x1B, 0x0A);        // Volume right
  (void) cs_write(0x1F, 0x0F);

  // Voodoo init sequence
  (void) cs_write(0x00, 0x99);
  (void) cs_write(0x47, 0x80);
  (void) cs_write(0x32, 0b10111011);
  (void) cs_write(0x32, 0b00111011);
  (void) cs_write(0x00, 0x00);

  i2s_init();

  (void) cs_write(0x02, 0b10011110);  // Power up

  return true;
}

static constexpr float pi = 3.1415926f;

static void note(float freq, float amp, unsigned duration) {
  float period = 48000 / freq;
  for (unsigned c = 0; c < duration / period; ++c) {
    for (float p = 0; p < period; ++p) {
      short sample = sinf(float(p) / period * 2*pi) * amp;
      // Left
      while (spi3.read_sr().get_txe() == false);
      spi3.write_dr(sample);
      // Right
      while (spi3.read_sr().get_txe() == false);
      spi3.write_dr(sample);
    }
  }
}

static constexpr float g = 392,
                       a = 440,
                       b = 493.88f,
                       c = 523.25f,
                       d = 587.33f,
                       e = 659.26f,
                       f = 698.46f;

static constexpr unsigned quarter = 24000;
static constexpr unsigned half = quarter * 2;

static struct {
  float freq;
  unsigned duration;
} const song[] = {
  { g, quarter },
  { c, quarter },
  { d, quarter },
  { e, half },
  { e, half },
  { 1, quarter },
  { e, quarter },
  { d, quarter },
  { e, quarter },
  { c, half },
  { c, half },
  { 1, quarter },
};

__attribute__((noreturn))
__attribute__((noinline))
static void rest() {
  vga::configure_timing(vga::timing_vesa_800x600_60hz);
  disable_irq(Interrupt::tim1_cc);
  disable_irq(Interrupt::tim8_cc);

  iic1_init();

  rcc.enable_clock(AhbPeripheral::gpiod);

  gpiod.set_output_type(Gpio::p4, Gpio::OutputType::push_pull);
  gpiod.set_output_speed(Gpio::p4, Gpio::OutputSpeed::medium_25mhz);
  gpiod.set_pull(Gpio::p4, Gpio::Pull::none);
  gpiod.set(Gpio::p4);
  gpiod.set_mode(Gpio::p4, Gpio::Mode::gpio);

  (void) cs_init();

  while (1) {
    for (auto s : song) {
      note(s.freq, 500, s.duration - 1000);
      note(s.freq, 0, 1000);
    }
  }

}

namespace vga {
  __attribute__((section(".ramcode")))
  void hblank_interrupt() {
    // iic1_poll();
  }
}

void etl_armv7m_reset_handler() {
  crt_init();
  vga::init();

  rest();
}


