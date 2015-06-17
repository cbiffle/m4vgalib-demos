c_library('lib',
  sources = [
    'table.cc',
    'tunnel.cc',
  ],
  local = {
    'cxx_flags': [
      '-O2',
      '-ffast-math',
    ],
  },
  deps = [
    '//demo',
    '//etl/armv7m',
    '//sys:libm',
    '//vga',
  ],
)

def demo(name, width, height):
  c_binary(name,
    environment = 'target',
    sources = [ 'main.cc' ],
    local = {
      # TODO(cbiffle): probably not necessary
      'cxx_flags': [ '-O2' ],
    },
    extra = {
      'cxx_flags': [
        '-DCFG_WIDTH=' + str(width),
        '-DCFG_HEIGHT=' + str(height),
      ],
    },
    deps = [
      ':lib',

      '//etl/armv7m:exception_table',
      '//etl/armv7m:implicit_crt0',
      '//etl/stm32f4xx:interrupt_table',
      '//runtime',
      '//runtime:default_traps',
      '//vga',
    ],
  )

demo('demo', 800, 600)
demo('demo_640', 640, 480)
