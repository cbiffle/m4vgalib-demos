c_library('math',
  sources = [
    'rand.cc',
  ],
  local = {
    'cxx_flags': [
      '-O2', '-ffast-math',
    ],
  },
  deps = [
    '//etl',
  ],
).extend_when(lambda e: e['arch'] != 'host',
  sources = [
    'trig.cc',
  ],
).extend_when(lambda e: e['arch'] == 'host',
  sources = [
    'trig_native.cc',
  ],
)
