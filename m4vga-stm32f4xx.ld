MEMORY {
  /* Alias of the SRAM112 region when it's remapped. */
  sram112_at_zero (rwx) : ORIGIN = 0x00000000, LENGTH =  112K

  rom (rx)      : ORIGIN = 0x08000000, LENGTH = 1024K
  ccm (rwx)     : ORIGIN = 0x10000000, LENGTH =   64K
  sram112 (rwx) : ORIGIN = 0x20000000, LENGTH =  112K
  sram16  (rwx) : ORIGIN = 0x2001c000, LENGTH =   16K
}

EXTERN(_ZN6armv7mL19v7m_exception_tableE)

ENTRY(v7m_reset_handler)

SECTIONS {
  .v7m_vector_table : {
    PROVIDE(_rom_vector_table_start = .);
    KEEP(*(.v7m_exception_table))
    KEEP(*(.stm32f4xx_interrupt_table))
    PROVIDE(_rom_vector_table_end = .);
  } >rom

  . = ALIGN(4);

  .text : {
    *(.text*)
    . = ALIGN(4);
    *(.rodata*)
    . = ALIGN(4);
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

  . = ALIGN(4);
  _etext = .;

  .data : AT (_etext) {
    _data = .;
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } >ccm

  . = ALIGN(4);

  .bss (NOLOAD) : {
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
  } >ccm

  .local (NOLOAD) : {
    KEEP(*(.vga_local_ram))
  } >ccm

  .ram_vectors (NOLOAD) : {
    . = ALIGN(512);
    PROVIDE(_ram_vector_table = .);
    . = . + SIZEOF(.v7m_vector_table);
  } >sram112_at_zero

  .sram16 (NOLOAD) : {
    KEEP(*(.vga_scan_ram))
  } >sram16
}

PROVIDE(v7m_initial_stack_top = ORIGIN(ccm) + LENGTH(ccm));
PROVIDE(_flash_base = ORIGIN(rom));
