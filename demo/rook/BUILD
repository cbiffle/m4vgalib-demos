c_library('lib',
  sources = [
    'rook.cc',
    '@demo/rook/model.cc',
  ],
  local = {
    'cxx_flags': [ '-O2', '-Igen' ],
  },
  deps = [
    '//demo',
    '//vga',
    '//sys:libm',
  ],
)

c_binary('demo',
  environment = 'target',
  sources = [ 'main.cc' ],
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

compile_stl('rook_stl_model',
  environment = 'base',
  stl_file = 'rook.stl',
)
