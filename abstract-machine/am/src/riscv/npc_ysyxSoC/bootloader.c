#include <string.h>

extern char _text_start[], _text_end[];
extern char _text_load_start[], _text_load_end[];

extern char _rodata_start[], _rodata_end[];
extern char _rodata_load_start[], _rodata_load_end[];


extern char _data_start[], _data_end[];
extern char _data_load_start[], _data_load_end[];
extern char _bss_start[], _bss_end[];

__attribute__((section(".boot")))
void *memcpy_boot(void *out, const void *in, size_t n) {
  char *d = out;
  const char *s = in;
  while (n--) {
    *d++ = *s++;
  }
  return out;
}

__attribute__((section(".boot")))
void *memset_boot(void *s, int c, size_t n) {
  char *p = s;
  while (n--) {
    *p++ = (char)c;
  }
  return s;
}

__attribute__((section(".boot")))
void bootloader() {
  // Copy data section from flash to psram
  if((void*)_data_start != (void*)_data_load_start) {
    memcpy_boot(_data_start, _data_load_start, _data_end - _data_start);
  }

  // Copy rodata section from flash to psram
  if((void*)_rodata_start != (void*)_rodata_load_start) {
    memcpy_boot(_rodata_start, _rodata_load_start, _rodata_end - _rodata_start);
  }

  // Copy text section from flash to psram
  if((void*)_text_start != (void*)_text_load_start) {
    memcpy_boot(_text_start, _text_load_start, _text_end - _text_start);
  }
  // Clear bss section
  memset_boot(_bss_start, 0, _bss_end - _bss_start);
}