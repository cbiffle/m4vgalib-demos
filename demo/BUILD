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
      '-DCFG_WIDTH=$width',
      '-DCFG_HEIGHT=$height',
      '-DCFG_HZ=$hz',
    ],
  },
  using = {
    'cxx_flags': [
      '-DCFG_WIDTH=$width',
      '-DCFG_HEIGHT=$height',
      '-DCFG_HZ=$hz',
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
