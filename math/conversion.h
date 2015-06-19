#ifndef MATH_CONVERSION_H
#define MATH_CONVERSION_H

namespace math {

#if __ARM_ARCH_7EM__ == 1
  /*
   * Optimized ceil and floor for ARMv7E-M, which as of this writing means
   * Cortex-M4.
   *
   * Turns out that backing up the FP rounding mode, overriding it, converting
   * to integer, and restoring the mode is super fast on this processor.  We
   * get by at one cycle per instruction for the sequence below.
   *
   * This might not prove to be true on the M7, but I'll worry about that when
   * I have one.
   *
   * Some notes on the implementation:
   *
   * - Our versions of ceil and floor return an int, not a float -- and thus
   *   return the result in a core, not VFP, register.  This is actually key to
   *   the performance: our calling convention can't return a twos-complement
   *   integer in a VFP register, and VFPv4-D16 doesn't have a float-rounding
   *   instruction.
   *
   * - They're inlined, not "for speed," but to make them cheap to use in leaf
   *   functions.  Since these routines produce nearly no register pressure
   *   (one core and one VFP register), they can be wormed into the caller's
   *   register allocation easily.  This keeps leaf routines leaf routines.
   *
   * - The constant materialization is done in C, rather than assembly, in the
   *   (so far futile) hope that the compiler would materialize the constant
   *   once for a sequence of conversions.  Disappointingly, GCC 4.8.3 misses
   *   the boat on this one, at least at -O2.
   *
   * - It makes me a little sad that a sequence of conversions repeatedly saves
   *   and restores the FPSCR, but this is so much better than what GCC
   *   provided that I can console myself.  I played with an alternative method
   *   for bracketing a routine with an FP mode change, but I can't quite get
   *   the dependency edges right in the asm block to keep GCC from moving the
   *   mode changes around in the output, breaking the semantics.
   */

  template <std::uint32_t rounding_mode>
  inline std::int32_t float_to_int_with_mode(float v) {
    // Implementation factor of ceil and floor.
    std::uint32_t old_fpscr;
    std::int32_t result;
    asm (
        "vmrs %[old_fpscr], FPSCR\n"
        "vmsr FPSCR, %[new_fpscr]\n"
        "vcvtr.s32.f32 %[v], %[v]\n"
        "vmsr FPSCR, %[old_fpscr]\n"
        "vmov %[result], %[v]\n"
      : [old_fpscr] "=&r"(old_fpscr),
        [v] "+&w"(v),
        [result] "=r"(result)
      : [new_fpscr] "r"(rounding_mode << 22)
    );
    return result;
  }

  inline std::int32_t floor(float v) {
    return float_to_int_with_mode<0b10>(v);
  }

  inline std::int32_t ceil(float v) {
    return float_to_int_with_mode<0b01>(v);
  }

#else

  // Defer to canned libc versions.
  std::int32_t floor(float v) {
    return std::int32_t(std::floor(v));
  }

  std::int32_t ceil(float v) {
    return std::int32_t(std::ceil(v));
  }

#endif


}  // namespace math

#endif  // MATH_CONVERSION_H
