#ifndef LIB_ARMV7M_COPY_WORDS_H
#define LIB_ARMV7M_COPY_WORDS_H

#include "lib/armv7m/types.h"

namespace armv7m {

/*
 * Moves some number of aligned words using the fastest method I could think up.
 */
void copy_words(Word const *source, Word *dest, Word count);

}  // namespace armv7m

#endif  // LIB_ARMV7M_COPY_WORDS_H
