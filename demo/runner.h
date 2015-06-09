#ifndef DEMO_RUNNER_H
#define DEMO_RUNNER_H

#include "etl/attribute_macros.h"
#include "vga/arena.h"
#include "demo/scene.h"

namespace demo {

/*
 * A function pointer that allocates and returns a Scene in the Arena.
 */
using Factory = vga::ArenaPtr<Scene> (*)();

template <typename T>
vga::ArenaPtr<Scene> make_scene() {
  return vga::arena_make<T>();
}

/*
 * Implementation factors of run, below, to keep most of the code out of this
 * header file.
 */
void general_setup();
void loop_setup();
void run_scene(Scene &);
bool start_button_pressed();

/*
 * Runs a sequence of Scene implementations one after another in an endless
 * loop.
 */
template <typename ... Ts>
ETL_NORETURN
void run(bool wait = false) {
  static constexpr Factory factories[sizeof...(Ts)] {
    make_scene<Ts>...
  };

  general_setup();
  while (wait && !start_button_pressed());

  while (true) {
    loop_setup();
    for (auto f : factories) {
      vga::arena_reset();
      auto scene = f();

      run_scene(*scene);
    }
  }
}

}  // namespace demo

#endif  // DEMO_RUNNER_H
