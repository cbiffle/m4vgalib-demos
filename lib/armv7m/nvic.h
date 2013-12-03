#ifndef LIB_ARMV7M_NVIC_H
#define LIB_ARMV7M_NVIC_H

#include "lib/armv7m/types.h"

namespace armv7m {

/*
 * ARMv7-M Nested Vectored Interrupt Controller (NVIC).
 *
 * Defined by section B3.4 of the ARMv7-M ARM.
 *
 * Remember that NVIC interrupt numbering starts with the first vendor-specific
 * interrupt (called "interrupt 0").  This also happens to be entry 16 in the
 * exception table ("exception 16").  Yes, this is a bit confusing.
 */

struct Nvic {
  /*
   * Interrupt {Set,Clear} Enable Registers.  These registers allow atomic
   * read-modify-write alterations to interrupt enable status.
   *
   * When writing to an ISER register, any 1 bits cause corresponding interrupts
   * to become enabled.  Reading from an ISER register reads a 1 for each
   * currently enabled interrupt, and zeroes for disabled interrupts.
   *
   * When writing to an ICER register, any 1 bits cause corresponding interrupts
   * to become disabled.
   *
   * In both cases, zero bits in the written value are ignored.
   *
   * ISER and ICER each form a bitfield, starting with the LSB of the first
   * register, which maps to interrupt 0.  The bit for interrupt n is given by:
   *
   *     iser[n / 32] & (1 << (n % 32))
   *     icer[n / 32] & (1 << (n % 32))
   *
   * A given ARMv7-M implementation may implement fewer than 16 IxER registers.
   * In general, the number of implemented registers on a part with n external
   * interrupts (i.e. not including exceptions) is ceil(n / 32).
   */
  Word volatile iser[16];
  Word const _reserved0[16];  // Future ISER expansion?  Not specified.

  Word volatile icer[16];
  Word const _reserved1[16];  // Future ICER expansion?  Not specified.

  /*
   * Interrupt {Set,Clear} Pending Registers.  These registers allow atomic
   * read-modify-write alterations to interrupt pending status.
   *
   * When writing to an ISPR register, any 1 bits cause corresponding interrupts
   * to become pending (i.e. will trigger when priority/interrupt mask next
   * permits).  Reading from an ISPR register reads a 1 for each currently
   * pending interrupt, and zeroes for non-pending interrupts.  Because pend
   * status is independent of enable status, this can be used to poll for
   * interrupts that are otherwise disabled.
   *
   * When writing to an ICPR register, any 1 bits cause corresponding interrupts
   * to become *not* pending (i.e. cancels a pending interrupt).
   *
   * In both cases, zero bits in the written value are ignored.
   *
   * ISPR and ICPR each form a bitfield, starting with the LSB of the first
   * register, which maps to interrupt 0.  The bit for interrupt n is given by:
   *
   *     ispr[n / 32] & (1 << (n % 32))
   *     icpr[n / 32] & (1 << (n % 32))
   *
   * A given ARMv7-M implementation may implement fewer than 16 IxPR registers.
   * In general, the number of implemented registers on a part with n external
   * interrupts (i.e. not including exceptions) is ceil(n / 32).
   */
  Word volatile ispr[16];
  Word const _reserved2[16];  // Future ISPR expansion?  Not specified.

  Word volatile icpr[16];
  Word const _reserved3[16];  // Future ICPR expansion?  Not specified.

  /*
   * Interrupt Active Bit Registers.  These registers allow inspection of which
   * interrupts are active (i.e. executing or preempted).
   *
   * Reading from an IAPR register reads a 1 for each currently active
   * interrupt, and zeroes for non-active interrupts.
   *
   * IAPR forms a bitfield, starting with the LSB of the first register, which
   * maps to interrupt 0.  The bit for interrupt n is given by:
   *
   *     iabr[n / 32] & (1 << (n % 32))
   *
   * A given ARMv7-M implementation may implement fewer than 16 IABR registers.
   * In general, the number of implemented registers on a part with n external
   * interrupts (i.e. not including exceptions) is ceil(n / 32).
   */
  Word const volatile iabr[16];
  Word const _reserved4[16];

  Word const _reserved5[32];

  /*
   * Interrupt Priority Registers.  These registers contain the priority level
   * for every external interrupt.
   *
   * Architectural priorities are 8-bit numbers, from 0 (most important) to
   * 255 (least important).  The IPR registers form an array of bytes, starting
   * with the least-significant byte of ipr[0], which holds the priority of
   * interrupt 0.  The byte for interrupt n is given by:
   *
   *     ipr[n / 4] & (0xFF << (8 * (n % 4))
   *
   * Unusually for an ARMv7-M system register, these registers explicitly
   * support byte access.  This provides a much easier way of accessing the
   * priority for interrupt n:
   *
   *     ipr_byte[n]
   *
   * A given ARMv7-M implementation may implement fewer than 124 IPR registers.
   * In general, the number of implemented registers on a part with n external
   * interrupts (i.e. not including exceptions) is ceil(n / 4).
   */
  union {
    Word volatile ipr[124];
    Byte volatile ipr_byte[124 * 4];
  };
};

/*
 * This gets provided by the linker script.
 */
extern Nvic nvic;

/*
 * Ensures that an IRQ (specified by number) is enabled.
 *
 * The change happens atomically: if the IRQ is pending when this function is
 * called, and priority allows it, the IRQ handler will execute before this
 * function returns.  This may seem like a corner case, but sometimes it's
 * useful to detect a pending event (using the ipsr registers above) and cause
 * the interrupt to fire at a controlled moment.
 *
 * You probably don't want to use this directly; the SoC driver layer should
 * provide a facade that takes a strongly typed enumeration.
 *
 * Precondition: the number is a valid IRQ number for this system.
 */
void enable_irq(unsigned);

/*
 * Ensures that an IRQ (specified by number) is enabled.
 *
 * The change happens atomically: once disable_irq returns, the interrupt is
 * disabled.  This may seem like an odd guarantee to state explicitly, but it's
 * remarkably easy to get this wrong.  This function gets it right so you can
 * think less.
 *
 * You probably don't want to use this directly; the SoC driver layer should
 * provide a facade that takes a strongly typed enumeration.
 *
 * Precondition: the number is a valid IRQ number for this system.
 */
void disable_irq(unsigned);

/*
 * Ensures that an IRQ (specified by number) is cleared.
 *
 * Typically called after the client has guaranteed that the interrupt can no
 * longer trigger, e.g. after shutting down a peripheral. Then, the interrupt
 * can safely be enabled with no risk of immediately triggering.
 *
 * You probably don't want to use this directly; the SoC driver layer should
 * provide a facade that takes a strongly typed enumeration.
 *
 * Precondition: the number is a valid IRQ number for this system.
 */
void clear_pending_irq(unsigned);

/*
 * Sets the priority for an IRQ (specified by number).
 *
 * The priority given here is the raw 8-bit ARMv7-M architectural priority.
 * Vendors may not implement some LSBs.  Normally the SoC layer will provide a
 * facade function that shifts to avoid aliasing.
 *
 * Precondition: the number is a valid IRQ number for this system.
 */
void set_irq_priority(unsigned, Byte);

}  // namespace armv7m

#endif  // LIB_ARMV7M_NVIC_H
