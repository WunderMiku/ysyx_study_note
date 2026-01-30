#include <am.h>
#include <klib-macros.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "riscv/npc_ysyxSoC/include/npc_ysyxSoC.h"

extern char _heap_start;
int main(const char *args);

extern char _pram_start;
#define PRAM_SIZE (8 * 1024) // 8KB form sram
#define PRAM_END  ((uintptr_t)&_pram_start + PRAM_SIZE)

Area heap = RANGE(&_heap_start, PRAM_END);
static const char mainargs[MAINARGS_MAX_LEN] = TOSTRING(MAINARGS_PLACEHOLDER); // defined in CFLAGS

void putch(char ch) {
  outb(SERIAL_PORT, ch);
}

void halt(int code) {
  npc_ysyxSoC_trap(code);
  // should not reach here
  while (1);
}

void print_int_as_chars(int value) {
  for (int i = 3; i >= 0; i--) {
      char c = (value >> (i * 8)) & 0xFF;
      printf("%c", c);
  }
}
static inline void print_msg(void) {
	unsigned int mvendorid, marchid;

	asm volatile ("csrr %0, mvendorid" : "=r"(mvendorid)); 
	asm volatile ("csrr %0, marchid" : "=r"(marchid));
	print_int_as_chars(mvendorid);
	printf("-%d\n", marchid);
}

extern char _data_start[], _data_end[];
extern char _data_load_start[], _data_load_end[];
extern char _bss_start[], _bss_end[];
void _trm_init() {
  // print_msg(); // For difftest: OFF needed
  if((void*)_data_start != (void*)_data_load_start) {
    memcpy(_data_start, _data_load_start, _data_end - _data_start);
  }

  memset(_bss_start, 0, _bss_end - _bss_start);

  int ret = main(mainargs);
  halt(ret);
}
