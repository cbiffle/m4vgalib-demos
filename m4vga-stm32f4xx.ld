MEMORY {
  rom (rx)      : ORIGIN = 0x08000000, LENGTH = 1024K
  ccm (rwx)     : ORIGIN = 0x10000000, LENGTH =   64K
  sram112 (rwx) : ORIGIN = 0x20000000, LENGTH =  112K
  sram16  (rwx) : ORIGIN = 0x2001c000, LENGTH =   16K
}

EXTERN(_ZN6armv7mL19v7m_exception_tableE)

ENTRY(v7m_reset_handler)

SECTIONS {
  .v7m_vector_table : {
    KEEP(*(.v7m_exception_table))
    KEEP(*(.stm32f4xx_interrupt_table))
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
}

PROVIDE(v7m_initial_stack_top = ORIGIN(ccm) + LENGTH(ccm));
