c_library('lib')

c_binary('demo',
  environment = 'target-standalone',
  sources = [ 'main.cc' ],
  deps = [
    ':lib',

    '//demo:terminal',

    '//etl/armv7m',
    '//etl/armv7m:exception_table',
    '//etl/stm32f4xx',
    '//etl/stm32f4xx:interrupt_table',
    '//etl/armv7m:manual_crt0',
    '//runtime',
    '//runtime:default_traps',
    '//vga',
  ],
)
