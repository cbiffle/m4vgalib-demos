MEMORY {
  /* Alias of the SRAM112 region when it's remapped. */
  sram112_at_zero (rwx) : ORIGIN = 0x00000000, LENGTH =  112K

  rom (rx)      : ORIGIN = 0x08000000, LENGTH = 1024K
  ccm (rwx)     : ORIGIN = 0x10000000, LENGTH =   64K
  sram112 (rwx) : ORIGIN = 0x20000000, LENGTH =  112K
  sram16  (rwx) : ORIGIN = 0x2001c000, LENGTH =   16K
}

EXTERN(_ZN3etl6armv7mL15exception_tableE)

ENTRY(etl_armv7m_reset_handler)

SECTIONS {
  /*
   * The sram_image section is loaded at the base of ROM.  Early in boot, we
   * copy it into SRAM to reduce access latency later.  This has an interesting
   * implication: while the vector table is position independent and can be
   * useful in both locations, the reset vector itself must not be in this
   * section!
   */
  .sram_image : {
    /* The vector table will exist in both locations after copy. */
    KEEP(*(.etl_armv7m_exception_table*))
    KEEP(*(.stm32f4xx_interrupt_table))
    /* Include any functions that have been explicitly placed here. */
    *(.ramcode*)
    /* Include vtables. */
    *(.rodata._ZTV*)
    /* Include initialized data here, so we initialize it during the copy. */
    *(.data*)
    /* We use a word copy, so ensure alignment. */
    . = ALIGN(4);
  } >sram112_at_zero AT>rom

  /* Generate some symbols to direct the copying code. */
  PROVIDE(_sram_image_start = LOADADDR(.sram_image));
  PROVIDE(_sram_image_end = _sram_image_start + SIZEOF(.sram_image));
  PROVIDE(_sram_image_dest = ORIGIN(sram112_at_zero));

  /*
   * Plain old .text contains functions without strict latency requirements,
   * and rodata.  This implies that const data goes in high-latency Flash,
   * and must be copied into RAM by any programs with latency requirements.
   */
  .text : {
    *(.text*)
    . = ALIGN(4);
    *(.rodata*)
  } >rom

  .init : ALIGN(2) {
    KEEP(*(.init_prologue))
    KEEP(*(.init))
    KEEP(*(.init_epilogue))
  } >rom

  .preinit_array : ALIGN(4) {
    _preinit_array_start = .;
    KEEP(*(.preinit_array))
    _preinit_array_end = .;
  } >rom

  .init_array : ALIGN(4) {
    _init_array_start = .;
    KEEP(*(.init_array))
    _init_array_end = .;
  } >rom

  .bss (NOLOAD) : {
    . = ALIGN(4);
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
  } >sram112_at_zero

  PROVIDE(_bss_start = ADDR(.bss));
  PROVIDE(_bss_end   = ADDR(.bss) + SIZEOF(.bss));

  .arena_sram112 (NOLOAD) : {
    . = ALIGN(4);
    PROVIDE(_sram112_arena_start = .);
    /* Exhaust the rest of this memory. */
    . = ORIGIN(sram112_at_zero) + LENGTH(sram112_at_zero);
  } >sram112_at_zero

  .local (NOLOAD) : {
    . += 1536;  /* Leave room for a stack. */
    PROVIDE(etl_armv7m_initial_stack_top = .);
    /* Allow stuff to be placed explicitly into CCM as needed. */
    *(.vga_local_ram)
    . = ALIGN(4);

    PROVIDE(_ccm_arena_start = .);
    . += 60000;  /* Encourage a minimum arena size. */
    /* Exhaust the rest of this memory. */
    . = ORIGIN(ccm) + LENGTH(ccm);
  } >ccm

  /* Allow stuff to be placed explicitly into scanout RAM as needed. */
  .sram16 (NOLOAD) : {
    *(.vga_scan_ram)
  } >sram16

  /*
   * Generate a table defining the boundaries of free RAM for the arenas. */
  .arena_regions : {
    PROVIDE(_arena_regions_start = .);
    LONG(_ccm_arena_start);
    LONG(ORIGIN(ccm) + LENGTH(ccm));
    LONG(_sram112_arena_start);
    LONG(ORIGIN(sram112_at_zero) + LENGTH(sram112_at_zero));
    PROVIDE(_arena_regions_end = .);
  } >rom

}

PROVIDE(_flash_base = ORIGIN(rom));
