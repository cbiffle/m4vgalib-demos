#ifndef BFF_INCLUDED_THROUGH_GENERATE
  #error Do not include biffield/generate_value_types.h directly, \
         use biffield/generate.h
#endif

/*
 * This file generates a type for each register, having the correct size and
 * accessors.
 */


/*******************************************************************************
 * For each register 'foo', generate 'foo_value_t', which is exactly the same
 * size as the access type but has typesafe accessors for manipulating register
 * contents.
 *
 * Note that the value types are intended to be used like integers.  Like the
 * number '1', you can't mutate them in place.  Just as '1 + 1' returns a new
 * integer (2), the register value types have *derivation operations* for
 * deriving new values.  These start with 'with_'.  For example:
 *
 *    value = value.with_lasers(none).with_kittens(30);
 *
 * Derivation operations are constexpr, so you can construct compile-time
 * literals this way.
 *
 * Value types also provide field accessors, whose names start with 'get_'.
 * Continuing our example:
 *
 *   size_t num_kittens = value.get_kittens();
 */

// Reserved registers require no support here.
#define BFF_REG_RESERVED(__at, __rn, __n)

// All registers -- read, write, array, etc. -- use common handling below.
#define BFF_REG_RW(__at, __rn, ...) \
  __BFF_REG_ANY(__at, __rn, __VA_ARGS__)
#define BFF_REG_RO(__at, __rn, ...) \
  __BFF_REG_ANY(__at, __rn, __VA_ARGS__)
#define BFF_REG_WO(__at, __rn, ...) \
  __BFF_REG_ANY(__at, __rn, __VA_ARGS__)


#define BFF_REG_ARRAY_RW(__at, __rn, __n, ...) \
  __BFF_REG_ANY(__at, __rn, __VA_ARGS__)

#define BFF_REG_ARRAY_WO(__at, __rn, __n, ...) \
  __BFF_REG_ANY(__at, __rn, __VA_ARGS__)

#define BFF_REG_ARRAY_RO(__at, __rn, __n, ...) \
  __BFF_REG_ANY(__at, __rn, __VA_ARGS__)

// Common register handling:
#define __BFF_REG_ANY(__at, __rn, ...) \
  class __rn ## _value_t { \
  public: \
    typedef __rn ## _meta meta_type;  /* from generate_constants.h */ \
    typedef __rn ## _value_t this_type;  /* handy self-reference type */ \
    typedef __at access_type;  /* shorter than accessing through meta_type */ \
    \
    /* explicit construction from access type */ \
    explicit constexpr __rn ## _value_t(access_type bits) : _bits(bits) {} \
    /* zero constructor with all bits clear */ \
    constexpr __rn ## _value_t() : _bits(0) {} \
    /* explicit cast to access type */ \
    explicit constexpr operator access_type() { return _bits; } \
    /* now evaluate the fields */ \
    __VA_ARGS__ \
  private: \
    access_type _bits;  /* actual member representing register contents */ \
  };


/*******************************************************************************
 * For each field 'bar' in register 'foo', generate:
 *
 *  - Derivation operation 'with_bar'.
 *  - Getter 'get_bar'.
 */

#define BFF_WAZ(__bf)

#define BFF_FIELD(__bf, __ft, __fn) __BFF_FIELD_COMMON(__bf, __ft, __fn)

// For enum types, additionally generate an enum class.
#define BFF_FIELD_E(__bf, __ft, __fn, __es) \
  enum class __fn ## _t : __ft { \
    __es \
  }; \
  __BFF_FIELD_COMMON(__bf, __fn ## _t, __fn)

#define BFF_ENUM(__v, __n) __n = __v,


#define __BFF_FIELD_COMMON(__bf, __ft, __fn) \
  /* derivation function */ \
  constexpr this_type with_ ## __fn(__ft value) { \
    return this_type(_bits \
        & ~meta_type::__fn::in_place_mask \
        | ((access_type(value) & meta_type::__fn::low_mask) \
               << meta_type::__fn::low_bit)); \
  } \
  \
  constexpr __ft get_ ## __fn() { \
    return __ft((_bits >> meta_type::__fn::low_bit) \
                & meta_type::__fn::low_mask); \
  }


#define BFF_FIELD_ARRAY(__bf, __n, __ft, __fn) \
  /* derivation function */ \
  constexpr this_type with_ ## __fn(unsigned idx, __ft value) { \
    return this_type(_bits \
        & ~meta_type::__fn::in_place_mask_of(idx) \
        | ((access_type(value) & meta_type::__fn::low_mask) \
               << meta_type::__fn::low_bit_of(idx))); \
  } \
  \
  constexpr __ft get_ ## __fn(unsigned idx) { \
    return __ft((_bits >> meta_type::__fn::low_bit_of(idx)) \
                & meta_type::__fn::low_mask); \
  }

/*******************************************************************************
 * Include the file and clean up.
 */

#include BFF_QUOTE(BFF_DEFINITION_FILE)

#undef BFF_ENUM

#undef BFF_FIELD_ARRAY
#undef BFF_FIELD_E
#undef BFF_FIELD
#undef __BFF_FIELD_COMMON
#undef BFF_WAZ

#undef BFF_REG_RW
#undef BFF_REG_WO
#undef BFF_REG_RO
#undef __BFF_REG_ANY

#undef BFF_REG_ARRAY_RW
#undef BFF_REG_ARRAY_WO
#undef BFF_REG_ARRAY_RO

#undef BFF_REG_RESERVED
