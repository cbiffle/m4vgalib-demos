#include "demo/input.h"

#include "etl/stm32f4xx/ahb.h"
#include "etl/stm32f4xx/rcc.h"
#include "etl/stm32f4xx/gpio.h"

using etl::stm32f4xx::AhbPeripheral;
using etl::stm32f4xx::rcc;
using etl::stm32f4xx::Gpio;
using etl::stm32f4xx::gpioa;
using etl::stm32f4xx::gpiob;

namespace demo {

void input_init() {
  rcc.enable_clock(AhbPeripheral::gpioa);
  rcc.enable_clock(AhbPeripheral::gpiob);

  gpioa.set_mode(Gpio::p0 | Gpio::p4 | Gpio::p6, Gpio::Mode::input);
  gpioa.set_pull(Gpio::p4 | Gpio::p6, Gpio::Pull::up);
  gpioa.set_pull(Gpio::p0, Gpio::Pull::down);

  gpiob.set_mode(Gpio::p14 | Gpio::p15, Gpio::Mode::input);
  gpiob.set_pull(Gpio::p13 | Gpio::p14 | Gpio::p15, Gpio::Pull::up);
}

bool user_button_pressed() {
  static bool last_pressed = false;

  bool current_pressed = gpioa.read_idr().get_id(0);

  bool p = current_pressed && (last_pressed != current_pressed);
  last_pressed = current_pressed;
  return p;
}

bool center_button_pressed() {
  static bool last_pressed = false;

  bool current_pressed = !gpiob.read_idr().get_id(13);

  bool p = current_pressed && (last_pressed != current_pressed);
  last_pressed = current_pressed;
  return p;
}

unsigned read_joystick() {
  unsigned state = 0;

  auto a = gpioa.read_idr();
  auto b = gpiob.read_idr();

  if (!a.get_id(4)) state |= JoyBits::left;
  if (!a.get_id(6)) state |= JoyBits::up;
  if (!b.get_id(15)) state |= JoyBits::down;
  if (!b.get_id(14)) state |= JoyBits::right;

  return state;
}

}  // namespace demo
