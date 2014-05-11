c_library('lib')  # TODO(cbiffle): present for consistency with Makefile

c_binary('demo',
  environment = 'target',
  sources = [ 'main.cc' ],
  extra = {
    'c_flags': [ '-nostdinc'],
    'cxx_flags': [ '-nostdinc' ],
    'link_flags': [ '-nostdlib', '-nodefaultlibs' ],
  },
  deps = [
    '//etl/armv7m',
    '//etl/armv7m:exception_table',
    '//etl/stm32f4xx:interrupt_table',
    '//etl/armv7m:manual_crt0',
    '//runtime',
    '//runtime:default_traps',
    '//vga',
    '//demo/horiz_tp:lib',
  ],
)
