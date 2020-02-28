c_library('lib',
  sources = [
    'map.cc',
    'raycast.cc',

    ':tex_gen#tex.cc',
  ],
  local = {
    'cxx_flags': [ '-O2' ],
  },
  deps = [
    ':tex_gen',
    '//demo',
    '//vga',
    '//sys:libm',
  ],
)

c_binary('demo',
  env = 'demo800',
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

convert_raycast_texture('tex_gen',
  env = 'base',
  tex_name = 'tex',
)
