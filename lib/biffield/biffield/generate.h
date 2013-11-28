/*
 * To produce register field, accessor, and type declarations, include this
 * file after defining BFF_DEFINITION_FILE.
 */
#define BFF_INCLUDED_THROUGH_GENERATE

#define BFF_QUOTE_(x) #x
#define BFF_QUOTE(x) BFF_QUOTE_(x)

/*
 * The order of these steps is significant.  See the individual files for
 * details.
 */
public:
#include "biffield/generate_constants.h"
#include "biffield/generate_value_types.h"
#include "biffield/generate_accessors.h"

private:
#include "biffield/checks.h"
#include "biffield/generate_fields.h"

public:

#undef BFF_QUOTE_
#undef BFF_QUOTE

#undef BFF_INCLUDED_THROUGH_GENERATE
