#include "etl/armv7m/implicit_crt0.h"

#include "demo/runner.h"
#include "demo/conway/conway.h"

int main() {
  demo::run<demo::conway::Conway>();
}
