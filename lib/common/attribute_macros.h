#ifndef LIB_KTL_ATTRIBUTE_MACROS_H
#define LIB_KTL_ATTRIBUTE_MACROS_H

#include "etl/common/attribute_macros.h"

// TODO(cbiffle): this is a temporary compatibility header!

#define NORETURN ETL_NORETURN

#define IMPLICIT /* implicit */

#define PACKED ETL_PACKED

#define SECTION(s) ETL_SECTION(s)

#define NAKED ETL_NAKED

#define USED ETL_USED

#define INLINE ETL_INLINE

#define ALIGNED(n) ETL_ALIGNED(n)

#endif  // LIB_KTL_ATTRIBUTE_MACROS_H
