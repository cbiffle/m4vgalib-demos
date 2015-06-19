c_library('lib',
  sources = [
    'raycast.cc',
  ],
  local = {
    'cxx_flags': [ '-O2' ],
  },
  deps = [
    '//demo',
    '//vga',
    '//sys:libm',
  ],
)

c_binary('demo',
  environment = 'demo800',
  sources = [ 'main.cc' ],
  deps = [
    ':lib',

    '//demo',
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
