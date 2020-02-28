c_library('lib',
  sources = [
    'rook.cc',
    ':rook_stl_model#model.cc',
  ],
  local = {
    'cxx_flags': [ '-O2', '-Igen' ],
  },
  deps = [
    # Ensure that the ordering dependency on model.h's generation gets
    # propagated to the compilation of rook.cc.
    ':rook_stl_model',

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

    '//etl/armv7m',
    '//etl/armv7m:exception_table',
    '//etl/stm32f4xx:interrupt_table',
    '//etl/armv7m:implicit_crt0',
    '//runtime',
    '//runtime:default_traps',
    '//vga',
  ],
  extra = {
    'cxx_flags': [ '-DDISRUPTIVE_MEASUREMENT' ],
  },
)

compile_stl('rook_stl_model',
  env = 'base',
  stl_file = 'rook.stl',
)
