ENTRY(_start)
/* PHDRS { text PT_LOAD; data PT_LOAD; } */

MEMORY {
  mrom  (rx)  : ORIGIN = 0x20000000, LENGTH = 4K
  flash (rx)  : ORIGIN = 0x30000000, LENGTH = 256M
  sram  (rwx) : ORIGIN = 0x0f000000, LENGTH = 8K
  psram (rwx) : ORIGIN = 0x80000000, LENGTH = 4M
}

SECTIONS {
  .boot : {
    _boot_start = .;
      *(entry)
      *(.boot*)
    _boot_end = .;
  } > flash

  .text : {
    _text_start = .;
    *(.text*)
    _text_end = .;
  } > sram AT> flash

  _text_load_start = LOADADDR(.text);
  _text_load_end = _text_load_start + SIZEOF(.text);

  etext = .;
  _etext = .;

  .rodata : {
    _rodata_start = .;
    *(.rodata*)
    _rodata_end = .;
  } > sram AT> flash

  _rodata_load_start = LOADADDR(.rodata);
  _rodata_load_end = _rodata_load_start + SIZEOF(.rodata);


  .data : {
    _data_start = .;
    *(.data*)
    *(.sdata*)
    *(.sdata.*)
    _data_end = .;
  } > sram AT> flash
  
  _data_load_start = LOADADDR(.data);
  _data_load_end = _data_load_start + SIZEOF(.data);

  .bss : {
	_bss_start = .;
    *(.bss*)
    *(.sbss*)
    *(.scommon)
  _bss_end = .;
    _stack_end = .;
  } > sram AT> flash

  _heap_start = ORIGIN(psram);

  _heap_end = ALIGN(ORIGIN(psram) + LENGTH(psram), 16);

  _stack_pointer = ALIGN(ORIGIN(sram) + LENGTH(sram) - 0x10, 16);
/*
   The stack grows downwards (towards lower addresses)
   and the stack pointer shall be aligned to a
   128-bit boundary upon procedure entry. 
*/
} 


