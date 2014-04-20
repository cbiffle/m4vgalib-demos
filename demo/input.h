#ifndef DEMO_INPUT_H
#define DEMO_INPUT_H

namespace demo {

void input_init();

bool user_button_pressed();

bool center_button_pressed();

enum JoyBits {
  up = 1,
  down = 2,
  left = 4,
  right = 8,
};

unsigned read_joystick();

}  // namespace demo

#endif  // DEMO_INPUT_H
