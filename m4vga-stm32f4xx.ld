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
    KEEP(*(.etl_stm32f4xx_interrupt_table))
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
  PROVIDE(_sram_image_end = LOADADDR(.sram_image) + SIZEOF(.sram_image));
  PROVIDE(_sram_image_dest = ORIGIN(sram112_at_zero));

  /*
   * Compatibility symbols for ETL crt0.  This exploits the fact that the SRAM
   * appears at two locations, so we can copy the SRAM image *before* remapping
   * by pretending it's initialized data.
   */
  PROVIDE(_data_init_image_start = LOADADDR(.sram_image));
  PROVIDE(_data_start = ORIGIN(sram112));
  PROVIDE(_data_end = ORIGIN(sram112) + SIZEOF(.sram_image));

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

  /*
   * We'd like software to be able to access bss using small addresses
   * around zero.  This ensures that such accesses are issued on the
   * processor's D bus, rather than having to go out the S bus.
   * However, at init time, we need to describe the BSS boundaries in
   * terms of the *pre-remap* address space.  The ridiculousness below
   * translates the address space shapes.
   */
  PROVIDE(_bss_start = ORIGIN(sram112)
                     + (ADDR(.bss) - ORIGIN(sram112_at_zero)));
  PROVIDE(_bss_end = ORIGIN(sram112)
                   + (ADDR(.bss) + SIZEOF(.bss) - ORIGIN(sram112_at_zero)));

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

    PROVIDE(_ccm_arena_end = ORIGIN(ccm) + LENGTH(ccm));
    LONG(_ccm_arena_start);
    LONG(_ccm_arena_end - _ccm_arena_start);

    PROVIDE(_sram112_arena_end = ORIGIN(sram112_at_zero)
                               + LENGTH(sram112_at_zero));
    LONG(_sram112_arena_start);
    LONG(_sram112_arena_end - _sram112_arena_start);
    PROVIDE(_arena_regions_end = .);
  } >rom

}

PROVIDE(_flash_base = ORIGIN(rom));
