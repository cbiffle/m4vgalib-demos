c_library('lib',
  sources = [
    'table.cc',
    'tunnel.cc',
  ],
  local = {
    'cxx_flags': [
      '-O2',
      '-ffast-math',
      '-DTABLE_IN_ROM=%(tunnel_table_in_rom)s',
    ],
  },
  deps = [
    '//demo',
    '//etl/armv7m',
    '//sys:libm',
    '//vga',
  ],
)

for name, mode in [('demo', 1), ('demo_online_tablegen', 0)]:
  c_binary(name,
    environment = 'target',
    sources = [ 'main.cc' ],
    local = {
      # TODO(cbiffle): probably not necessary
      'cxx_flags': [ '-O2' ],
    },
    extra = {
      'tunnel_table_in_rom': mode,
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
