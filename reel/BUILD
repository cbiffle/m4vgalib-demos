c_binary('reel',
  environment = 'target',
  sources = [ 'main.cc' ],
  extra = {
    'tunnel_table_in_rom': 0,
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

    '//demo/tunnel:lib',
    '//demo/conway:lib',
    '//demo/hires_text:lib',
    '//demo/hires_mix:lib',
    '//demo/rook:lib',
    '//demo/xor_pattern:lib',
  ],
)
