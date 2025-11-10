#include <am.h>
#include <klib-macros.h>
#include <stdio.h>
#include "riscv/npc/include/npc.h"

extern char _heap_start;
int main(const char *args);

extern char _pmem_start;
#define PMEM_SIZE (128 * 1024 * 1024)
#define PMEM_END  ((uintptr_t)&_pmem_start + PMEM_SIZE)

Area heap = RANGE(&_heap_start, PMEM_END);
static const char mainargs[MAINARGS_MAX_LEN] = TOSTRING(MAINARGS_PLACEHOLDER); // defined in CFLAGS

void putch(char ch) {
  outb(SERIAL_PORT, ch);
}

void halt(int code) {
  npc_trap(code);
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

void _trm_init() {
  print_msg(); // For difftest: OFF need
  int ret = main(mainargs);
  halt(ret);
}
