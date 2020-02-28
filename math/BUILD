c_library('math',
  sources = [
    'rand.cc',
    'trig_$arch.cc',
  ],
  local = {
    'cxx_flags': [
      '-O2', '-ffast-math',
    ],
  },
  deps = [
    '//etl',
  ],
)
