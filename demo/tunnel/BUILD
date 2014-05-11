c_library('lib',
  sources = [
    'tunnel.cc',
  ],
  local = {
    'cxx_flags': [ '-O2' ],
  },
  deps = [
    '//demo',
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
  extra = {
    'link_flags': [ '-nostartfiles' ],
  },
  deps = [
    ':lib',

    '//etl/armv7m:exception_table',
    '//etl/armv7m:manual_crt0',
    '//etl/stm32f4xx:interrupt_table',
    '//runtime',
    '//runtime:default_traps',
    '//vga',
  ],
)
