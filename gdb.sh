#!/bin/sh

# Intended use:
#   $ gdb.sh build/latest/demo/rook/demo
# Note that you must have started openocd separately for this to work.
# Once inside gdb, the gdbconfig gives you a 'flash' command.

ROOT="$(dirname "$0")"

arm-none-eabi-gdb -x "$ROOT/gdbconfig" $1
