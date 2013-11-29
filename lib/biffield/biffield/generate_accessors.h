#ifndef BFF_INCLUDED_THROUGH_GENERATE
  #error Do not include biffield/generate_accessors.h directly, \
         use biffield/generate.h
#endif

/*
 * Generate typesafe accessors for registers.
 */


/*******************************************************************************
 * For each register 'foo', generate some combination of:
 *
 *  - 'read_foo()' if the register can be read.
 *  - 'write_foo(foo_value_t)' if the register can be written.
 *
 * Arrays of registers take an additional index argument to both read and write.
 */

#define BFF_REG_RESERVED(__at, __rn, __n)


#define BFF_REG_ARRAY_RW(__at, __rn, __n, __fs) \
  BFF_REG_ARRAY_RO(__at, __rn, __n, __fs) \
  BFF_REG_ARRAY_WO(__at, __rn, __n, __fs)

#define BFF_REG_ARRAY_RO(__at, __rn, __n, __fs) \
  __rn ## _value_t read_##__rn(unsigned index) { \
    return __rn ## _value_t(_##__rn[index]); \
  }

#define BFF_REG_ARRAY_WO(__at, __rn, __n, __fs) \
  void write_##__rn(unsigned index, __rn##_value_t value) { \
    _##__rn[index] = __at(value); \
  }


#define BFF_REG_RW(__at, __n, __fs) \
  BFF_REG_RO(__at, __n, __fs) \
  BFF_REG_WO(__at, __n, __fs)

#define BFF_REG_RO(__at, __n, __fs) \
  __n ## _value_t read_##__n() { \
    return __n ## _value_t(_##__n); \
  }

#define BFF_REG_WO(__at, __n, __fs) \
  void write_##__n(__n##_value_t value) { \
    _##__n = __at(value); \
  } \
  \
  void write_##__n(__at value) { \
    _##__n = value; \
  } \


/*******************************************************************************
 * Include the file and clean up.
 */

#include BFF_QUOTE(BFF_DEFINITION_FILE)

#undef BFF_REG_RW
#undef BFF_REG_RO
#undef BFF_REG_WO

#undef BFF_REG_ARRAY_RW
#undef BFF_REG_ARRAY_WO
#undef BFF_REG_ARRAY_RO

#undef BFF_REG_RESERVED
