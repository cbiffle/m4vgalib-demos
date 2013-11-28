#include "lib/armv7m/exception_table.h"
#include "lib/common/attribute_macros.h"

namespace armv7m {

/*******************************************************************************
 * The exception table!
 */

typedef void (*ExceptionHandler)(void);

struct ExceptionTable {
  Word const *initial_stack_top;
  ExceptionHandler reset_handler;

  #define V7M_EXCEPTION(name) ExceptionHandler name ## _handler;
  #define V7M_EXCEPTION_RESERVED(n) ExceptionHandler __reserved ## n;
  #include "lib/armv7m/exceptions.def"
  #undef V7M_EXCEPTION
  #undef V7M_EXCEPTION_RESERVED
};

static_assert(sizeof(ExceptionTable) == 16 * sizeof(Word),
              "ExceptionTable size is wrong.");

/*
 * There are a couple of nuances to the exception table definition.
 *
 *  1. We place it in a section called .v7m_exception_table.  There's nothing
 *     special about that name, except that the linker scripts look for it and
 *     put it in the right place.
 *
 *  2. We make the table const, ensuring that the linker will let us place it
 *     in Flash if the linker script wants it there.
 *
 *  3. Of course, this table doesn't include any vendor-specific interrupt
 *     vectors, which are typically tacked on right at the end using a separate
 *     section in the linker script.
 */
SECTION(".v7m_exception_table")
USED
ExceptionTable const v7m_exception_table = {
  &v7m_initial_stack_top,
  v7m_reset_handler,

  #define V7M_EXCEPTION(name) v7m_##name##_handler,
  #define V7M_EXCEPTION_RESERVED(n) (ExceptionHandler) 0,
  #include "lib/armv7m/exceptions.def"
  #undef V7M_EXCEPTION
  #undef V7M_EXCEPTION_RESERVED
};

}  // namespace armv7m
