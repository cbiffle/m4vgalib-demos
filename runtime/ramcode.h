#ifndef RUNTIME_RAMCODE_H
#define RUNTIME_RAMCODE_H

namespace runtime {

/*
 * Call this shortly after the crt0 to remap memory and place selected code
 * in RAM.
 */
void ramcode_init();

}  // namespace runtime

#endif  // RUNTIME_RAMCODE_H
