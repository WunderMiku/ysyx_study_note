ENTRY(_start)
/* PHDRS { text PT_LOAD; data PT_LOAD; } */

MEMORY {
  mrom (rx)  : ORIGIN = 0x20000000, LENGTH = 4K
  sram (rwx) : ORIGIN = 0x0f000000, LENGTH = 8K
}

SECTIONS {
  .text : {
    *(entry)
    *(.text*)
  } > mrom 

  etext = .;
  _etext = .;

  .rodata : {
    *(.rodata*)
  } > mrom


  .data : {
    _data_start = .;
    *(.data*)
    *(.sdata*)
    *(.sdata.*)
    _data_end = .;
  } > sram AT> mrom
  
  _data_load_start = LOADADDR(.data);
  _data_load_end = _data_load_start + SIZEOF(.data);

  .bss : {
	_bss_start = .;
    *(.bss*)
    *(.sbss*)
    *(.scommon)
  _bss_end = .;
  } > sram AT> mrom

  _heap_start = ALIGN(8);

  _stack_pointer = ORIGIN(sram) + LENGTH(sram) - 0x4;
  _stack_end = _stack_pointer - 0x1000;
  _heap_end = _stack_end - 0x40;
} 


