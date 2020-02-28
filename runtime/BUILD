c_library('runtime',
  sources = [
    'runtime.cc',
    'startup.cc',
  ],
  deps = [
    '//etl:etl',
    '//etl/armv7m:armv7m',
    '//etl/stm32f4xx:stm32f4xx',
  ],
  local = {
    # startup.cc contains startup code that gets added to the preinit_array.
    # If this code is packaged into a static archive, the linker won't grab
    # the preinit_array table, and thus won't link in the startup routines,
    # without setting whole_archive.
    'c_library_whole_archive': True,
  }
)

c_library('default_traps',
  sources = [ 'traps.cc' ],
  deps = [
    '//etl/armv7m',
  ],
  local = {
    # We don't correctly represent the dependency edge from the ARMv7M
    # vector table (provided by ETL) to this target, which provides the
    # traps.  We have to use whole_archive to hack around this.
    'c_library_whole_archive': True,
  },
)
