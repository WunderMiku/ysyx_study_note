ENTRY(_start)
/* PHDRS { text PT_LOAD; data PT_LOAD; } */

MEMORY {
  mrom (rx)  : ORIGIN = 0x20000000, LENGTH = 4K
  flash (rx) : ORIGIN = 0x30000000, LENGTH = 256M
  sram (rwx) : ORIGIN = 0x0f000000, LENGTH = 8K
}

SECTIONS {
  .text : {
    *(entry)
    *(.text*)
  } > flash 

  etext = .;
  _etext = .;

  .rodata : {
    *(.rodata*)
  } > flash


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
  _heap_start = ALIGN(8);
  } > sram AT> flash

  _stack_pointer = ALIGN(ORIGIN(sram) + LENGTH(sram) - 0x10, 16);
/*
   The stack grows downwards (towards lower addresses)
   and the stack pointer shall be aligned to a
   128-bit boundary upon procedure entry. The 
   first argument passed on the stack is located
   at offset zero of the stack pointer on function
   entry; following arguments are stored at correspondingly
   higher addresses.
*/
  _stack_end = _stack_pointer - 0x1000;

  _heap_end = _stack_end - 0x40;
} 


