c_library('lib')  # TODO(cbiffle): present for consistency with Makefile

c_binary('demo',
  env = 'demo800',
  sources = [ 'main.cc' ],
  deps = [
    '//etl/armv7m',
    '//etl/armv7m:exception_table',
    '//etl/stm32f4xx:interrupt_table',
    '//etl/armv7m:implicit_crt0',
    '//runtime',
    '//runtime:default_traps',
    '//vga',
    '//demo/horiz_tp:lib',
  ],
)
