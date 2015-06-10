#include "etl/armv7m/implicit_crt0.h"

#include "demo/runner.h"
#include "demo/hires_text/hires_text.h"

int main() {
  demo::run<demo::hires_text::HiresText>();
}
