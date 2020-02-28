c_library('lib',
  sources = [
    'rotozoom.cc',
  ],
  local = {
    'cxx_flags': [
      '-O2',
      '-ffast-math',
    ],
  },
  deps = [
    '//demo',
    '//etl/armv7m',
    '//sys:libm',
    '//vga',
  ],
)

c_binary('demo',
  env = 'demo800',
  sources = [ 'main.cc' ],
  local = {
    # TODO(cbiffle): probably not necessary
    'cxx_flags': [ '-O2' ],
  },
  deps = [
    ':lib',

    '//etl/armv7m:exception_table',
    '//etl/armv7m:implicit_crt0',
    '//etl/stm32f4xx:interrupt_table',
    '//runtime',
    '//runtime:default_traps',
    '//vga',
  ],
)
