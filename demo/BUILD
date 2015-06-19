c_library('demo',
  sources = [
    'input.cc',
    'runner.cc',
  ],
  deps = [
    '//etl/stm32f4xx',
  ],
  local = {
    'cxx_flags': [
      '-DCFG_WIDTH=%(width)s',
      '-DCFG_HEIGHT=%(height)s',
      '-DCFG_HZ=%(hz)s',
    ],
  },
  using = {
    'cxx_flags': [
      '-DCFG_WIDTH=%(width)s',
      '-DCFG_HEIGHT=%(height)s',
      '-DCFG_HZ=%(hz)s',
    ],
  },
)

c_library('terminal',
  sources = [
    'terminal.cc',
  ],
  deps = [
    '//etl/stm32f4xx',
    '//vga',
  ],
)
