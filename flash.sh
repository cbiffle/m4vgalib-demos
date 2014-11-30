#!/bin/sh -e

# Intended use:
#   $ ./flash.sh build/latest/demo/rook/demo
# The openocd.cfg is specific to the STLink/V2 programmer, like the one found
# on the STM32F4DISCOVERY board.
#
# Note that openocd support for this programmer is a little flaky at times, and
# if you need to flash repeatedly, you'll have better luck running a single
# continuous openocd session and using gdb.sh.

ROOT="$(dirname "$0")"

openocd -f "$ROOT/openocd.cfg" \
  -c "init" \
  -c "reset init" \
  -c "flash write_image erase $1" \
  -c "reset halt" \
  -c "resume" \
  -c "shutdown"
