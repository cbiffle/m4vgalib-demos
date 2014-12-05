c_library('lib',
  sources = [
    'conway.cc',
  ],
  local = {
    # Best performance for this module is *not* obtained at -Os... nor,
    # interestingly enough, at -O2/O3.  (This is true as of GCC 4.6.3.)
    'cxx_flags': [ '-O1' ],
  },
  deps = [
    '//demo',
    '//vga',
  ],
)

c_binary('demo',
  environment = 'target-standalone',
  sources = [ 'main.cc' ],
  deps = [
    ':lib',

    '//etl',
    '//etl/armv7m',
    '//etl/armv7m:exception_table',
    '//etl/stm32f4xx:interrupt_table',
    '//etl/armv7m:implicit_crt0',
    '//runtime',
    '//runtime:default_traps',
    '//vga',
  ],
)
