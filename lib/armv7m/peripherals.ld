/*
 * This linker script assigns addresses to the common peripherals specified by
 * the ARMv7-M architecture.
 *
 * The symbols on the left are, unfortunately, mangled.  This is because:
 *  1. We don't want to put the peripheral names in the "extern C" global
 *     namespace, for hygenic reasons.
 *  2. gnu ld doesn't understand gnu C++'s mangling convention.
 *
 * Do not be alarmed by the mangling -- you can do it by hand because the
 * symbols used here are simple extern objects.  The pattern is:
 *  - Literal "_ZN"
 *  - Each component of the namespace, prefixed with its length in decimal
 *  - Literal "E"
 *
 * As a result, if you pretend that any digit is really "::", you can read
 * the mangled symbols below.  If you find this annoying, pipe the file
 * through c++filt.
 */

_ZN6armv7m4nvicE = 0xe000e100;
_ZN6armv7m3scbE  = 0xe000ed00;
