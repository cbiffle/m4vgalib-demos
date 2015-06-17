included_demos = [
  'conway',
  'hires_mix',
  'hires_text',
  'rook',
  'tunnel',
  'wipe',
  'xor_pattern',
]

c_binary('reel',
  environment = 'target',
  sources = [ 'main.cc' ],
  extra = {
    'tunnel_table_in_rom': 0,
    'cxx_flags': [
      '-DCFG_WIDTH=800',
      '-DCFG_HEIGHT=600',
    ],
  },
  deps = [
    '//etl/armv7m:armv7m',
    '//etl/armv7m:exception_table',
    '//etl/armv7m:implicit_crt0',
    '//etl/stm32f4xx:interrupt_table',

    '//runtime:runtime',
    '//runtime:default_traps',
    '//vga:vga',
    '//demo:demo',
  ] + ['//demo/%s:lib' % d for d in included_demos],
)
