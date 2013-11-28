#ifndef BFF_INCLUDED_THROUGH_GENERATE
  #error Do not include biffield/generate_fields.h directly, \
         use biffield/generate.h
#endif

// This is non-const to dodge a GCC warning.  Believe it!
#define BFF_REG_RESERVED(__at, __rn, __n) __at _reserved_##__rn[__n];


#define BFF_REG_ARRAY_RW(__at, __rn, __n, __fs) \
  __at volatile _##__rn[__n];

// This is non-const to dodge a GCC warning.  Believe it!
#define BFF_REG_ARRAY_RO(__at, __rn, __n, __fs) \
  __at volatile _##__rn[__n];

#define BFF_REG_ARRAY_WO(__at, __rn, __n, __fs) \
  __at volatile _##__rn[__n];


#define BFF_REG_RW(__at, __n, __fs) \
  __at volatile _##__n;

#define BFF_REG_RO(__at, __n, __fs) \
  __at volatile _## __n;

#define BFF_REG_WO(__at, __n, __fs) \
  __at volatile _##__n;

#include BFF_QUOTE(BFF_DEFINITION_FILE)

#undef BFF_REG_RW
#undef BFF_REG_RO
#undef BFF_REG_WO

#undef BFF_REG_ARRAY_RW
#undef BFF_REG_ARRAY_WO
#undef BFF_REG_ARRAY_RO

#undef BFF_REG_RESERVED
