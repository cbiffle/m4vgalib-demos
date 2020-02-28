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

def demo(name, env):
  c_binary(name,
    env = env,
    sources = [ 'main.cc' ],
    local = {
      # TODO(cbiffle): probably not necessary
      'cxx_flags': [ '-O2' ],
    },
    deps = [
      ':lib',

      '//etl/armv7m:exception_table',
      '//etl/armv7m:implicit_crt0',
      '//etl/stm32f4xx:interrupt_table',
      '//demo',
      '//runtime',
      '//runtime:default_traps',
      '//vga',
    ],
  )

demo('demo', 'demo800')
demo('demo_640', 'demo640')
