#include "lib/common/range_ptr.h"

#include "lib/armv7m/exception_table.h"

#include "lib/stm32f4xx/apb.h"
#include "lib/stm32f4xx/gpio.h"
#include "lib/stm32f4xx/interrupts.h"
#include "lib/stm32f4xx/rcc.h"
#include "lib/stm32f4xx/iic.h"

#include "runtime/startup.h"

#include "vga/timing.h"
#include "vga/vga.h"

using common::RangePtr;

using stm32f4xx::gpiob;
using stm32f4xx::gpiod;
using stm32f4xx::rcc;
using stm32f4xx::Iic;
using stm32f4xx::iic1;
using stm32f4xx::Gpio;

static void iic1_init() {
  // Turn on the IIC unit.
  rcc.enable_clock(stm32f4xx::ApbPeripheral::i2c1);

  float clock_hz = rcc.get_clock_hz(stm32f4xx::ApbPeripheral::i2c1);
  iic1.write_cr2(Iic::cr2_value_t()
                 .with_freq(clock_hz / 1e6f));
  iic1.write_ccr(Iic::ccr_value_t()
                 .with_fs(false)
                 .with_ccr(clock_hz / (2 * 100e3f)));

  iic1.write_trise(Iic::trise_value_t()
                   .with_trise(static_cast<unsigned>(clock_hz / 1e6f) + 1));

  iic1.write_cr1(Iic::cr1_value_t().with_pe(true));

  // We'll be using GPIOB for IIC.
  rcc.enable_clock(stm32f4xx::AhbPeripheral::gpiob);

  unsigned short const iic_pins = Gpio::p6 | Gpio::p9;
  gpiob.set_output_type(iic_pins, Gpio::OutputType::open_drain);
  gpiob.set_output_speed(iic_pins, Gpio::OutputSpeed::medium_25mhz);
  gpiob.set_pull(iic_pins, Gpio::Pull::none);
  gpiob.set_alternate_function(iic_pins, 4);
  gpiob.set_mode(iic_pins, Gpio::Mode::alternate);
}

static bool iic1_ping(unsigned address) {
  // Set START
  iic1.write_cr1(iic1.read_cr1()
                 .with_start(true));
  // Wait for SB
  while (iic1.read_sr1().get_sb() == false);
  // Write address
  iic1.write_dr(address << 1);
  // Wait for ADDR, read SR1 and SR2
  bool responded = false;
  while (true) {
    auto sr1 = iic1.read_sr1();
    if (sr1.get_addr()) {
      responded = true;
      break;
    }

    if (sr1.get_af()) {
      responded = false;
      break;
    }
  }

  // Stop
  iic1.write_cr1(iic1.read_cr1().with_stop(true));
  while (iic1.read_cr1().get_stop() == true);

  return responded;
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

static bool cs_init() {

  return cs_write(0x00, 0x99)
      && cs_write(0x47, 0x80)
      && cs_write(0x32, 0b10111011)
      && cs_write(0x32, 0b00111011)
      && cs_write(0x00, 0x00);
}

void v7m_reset_handler() {
  crt_init();
  vga::init();

  vga::configure_timing(vga::timing_vesa_800x600_60hz);

  iic1_init();

  rcc.enable_clock(stm32f4xx::AhbPeripheral::gpiod);

  gpiod.set_output_type(Gpio::p4, Gpio::OutputType::push_pull);
  gpiod.set_output_speed(Gpio::p4, Gpio::OutputSpeed::medium_25mhz);
  gpiod.set_pull(Gpio::p4, Gpio::Pull::none);
  gpiod.set(Gpio::p4);
  gpiod.set_mode(Gpio::p4, Gpio::Mode::gpio);

  (void) cs_init();
  
  while (1) {
    iic1_ping(0x4a);
  }

}

namespace vga {
  __attribute__((section(".ramcode")))
  void hblank_interrupt() {
    // iic1_poll();
  }
}
