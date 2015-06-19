c_library('lib',
  sources = [ 'hires_text.cc' ],
  deps = [
    '//demo',
    '//demo:terminal',
    '//vga',
  ],
)

c_binary('demo',
  environment = 'demo800',
  sources = [ 'main.cc' ],
  deps = [
    ':lib',

    '//etl/armv7m',
    '//etl/armv7m:exception_table',
    '//etl/stm32f4xx:interrupt_table',
    '//etl/armv7m:implicit_crt0',
    '//runtime',
    '//runtime:default_traps',
    '//vga',
  ],
)
