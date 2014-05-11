c_library('lib',
  sources = [ 'hires_text.cc' ],
  deps = [
    '//demo',
    '//vga',
  ],
)

c_binary('demo',
  environment = 'target',
  sources = [ 'main.cc' ],
  extra = {
    'cxx_flags': [ '-nostdinc' ],
    'link_flags': [ '-nostdlib', '-nodefaultlibs' ],
  },
  deps = [
    ':lib',

    '//etl/armv7m',
    '//etl/armv7m:exception_table',
    '//etl/stm32f4xx:interrupt_table',
    '//etl/armv7m:manual_crt0',
    '//runtime',
    '//runtime:default_traps',
    '//vga',
  ],
)
