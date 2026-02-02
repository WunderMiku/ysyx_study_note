#include <string.h>

extern char _data_start[], _data_end[];
extern char _data_load_start[], _data_load_end[];
extern char _bss_start[], _bss_end[];
void bootloader() {
  // Copy data section from flash to sram
  if((void*)_data_start != (void*)_data_load_start) {
    memcpy(_data_start, _data_load_start, _data_end - _data_start);
  }
  // Clear bss section
  memset(_bss_start, 0, _bss_end - _bss_start);
}