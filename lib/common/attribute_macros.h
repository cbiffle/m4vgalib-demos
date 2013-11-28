#ifndef LIB_KTL_ATTRIBUTE_MACROS_H
#define LIB_KTL_ATTRIBUTE_MACROS_H

#define NORETURN __attribute__((noreturn))

#define IMPLICIT /* implicit */

#define PACKED __attribute__((packed))

#define SECTION(s) __attribute__((section(s)))

#define NAKED __attribute__((naked))

#define USED __attribute__((used))

#define INLINE inline __attribute__((always_inline))

#endif  // LIB_KTL_ATTRIBUTE_MACROS_H
