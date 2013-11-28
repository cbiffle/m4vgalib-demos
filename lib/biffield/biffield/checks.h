#ifndef BFF_INCLUDED_THROUGH_GENERATE
  #error Do not include biffield/checks.h directly, use biffield/generate.h
#endif

/*
 * This file generates code to perform compile-time checks on the validity of
 * bitfield definitions.
 */


/*******************************************************************************
 * Generate static_asserts to check the integrity of the register definitions.
 */

// Disregard reserved registers.
#define BFF_REG_RESERVED(__at, __rn, __n)


// All arrays are handled in a common way...
#define BFF_REG_ARRAY_RW(__at, __rn, __n, __fs) \
  __BFF_REG_ARRAY_ANY(__at, __rn, __n, __fs)
#define BFF_REG_ARRAY_WO(__at, __rn, __n, __fs) \
  __BFF_REG_ARRAY_ANY(__at, __rn, __n, __fs)
#define BFF_REG_ARRAY_RO(__at, __rn, __n, __fs) \
  __BFF_REG_ARRAY_ANY(__at, __rn, __n, __fs)

// ... which currently is shared with the general register handling below.
#define __BFF_REG_ARRAY_ANY(__at, __rn, __n, __fs) \
  __BFF_REG_ANY(__at, __rn, __fs)


// All registers are handled in a common way, below.
#define BFF_REG_RW(__at, __rn, __fs) __BFF_REG_ANY(__at, __rn, __fs)
#define BFF_REG_RO(__at, __rn, __fs) __BFF_REG_ANY(__at, __rn, __fs)
#define BFF_REG_WO(__at, __rn, __fs) __BFF_REG_ANY(__at, __rn, __fs)

/*
 * For a register named 'foo', generate a (private nested) type
 * 'foo_bitfield_checks' where we can define some things and assert on their
 * relationships.
 */
#define __BFF_REG_ANY(__at, __rn, __fs) \
  struct __rn ## _bitfield_checks { \
    typedef __rn ## _meta meta_type; \
    __fs \
  };


/*******************************************************************************
 * Within each register, generate static_asserts on the field definitions.
 */

// Both normal and enum fields are handled the same way.
#define BFF_FIELD(__bf, __ft, __fn) __BFF_FIELD_COMMON(__bf, __ft, __fn, )

#define BFF_FIELD_E(__bf, __ft, __fn, __es) \
  __BFF_FIELD_COMMON(__bf, __ft, __fn, __es)

#define BFF_FIELD_ARRAY(__bf, __n, __ft, __fn) \
  __BFF_FIELD_COMMON(__bf, __ft, __fn, \
    static_assert(f_meta::bit_count % __n == 0, \
                  "Array field " #__fn " not an integer number of elements."); \
  )

/*
 * For a field 'bar' in register 'foo', generate nested type
 * 'foo_bitfield_checks::bar' containing checks.  This marginally improves
 * error reporting.
 */
#define __BFF_FIELD_COMMON(__bf, __ft, __fn, __extra) \
  struct __fn { \
    typedef meta_type::__fn f_meta; \
    static_assert(f_meta::high_bit >= f_meta::low_bit, \
                  "hi/lo bits for " #__fn " bad (backwards?)"); \
    static_assert(f_meta::high_bit < sizeof(meta_type::access_type) * 8, \
                  "hi bit for " #__fn " out of range for type"); \
    __extra \
  };

// Don't bother checking WAZ fields for now.
#define BFF_WAZ(x)

// With 'mask' created above, check that enum constants are valid.
#define BFF_ENUM(__v, __n) \
  static_assert((__v) == ((__v) & f_meta::low_mask), \
                "too many bits in enum " #__n);


/*******************************************************************************
 * Include the file and clean up.
 */

#include BFF_QUOTE(BFF_DEFINITION_FILE)

#undef BFF_ENUM

#undef __BFF_FIELD_COMMON
#undef BFF_FIELD_ARRAY
#undef BFF_FIELD_E
#undef BFF_FIELD
#undef BFF_WAZ

#undef BFF_REG_RW
#undef BFF_REG_WO
#undef BFF_REG_RO
#undef __BFF_REG_ANY

#undef BFF_REG_ARRAY_RW
#undef BFF_REG_ARRAY_WO
#undef BFF_REG_ARRAY_RO
#undef __BFF_REG_ARRAY_ANY

#undef BFF_REG_RESERVED
