c_library('lib',
  sources = [
    'wipe.cc',
  ],
  local = {
    'cxx_flags': [
      '-O2',
      '-ffast-math',
    ],
  },
  deps = [
    '//demo/xor_pattern:lib',

    '//demo',
    '//demo:terminal',
    '//etl/armv7m',
    '//sys:libm',
    '//vga',
  ],
)

c_binary('demo',
  environment = 'target',
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
