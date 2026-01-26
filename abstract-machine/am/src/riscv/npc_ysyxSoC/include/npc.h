#include "riscv/riscv.h"

# define npc_trap(code) asm volatile("mv a0, %0; ebreak" : :"r"(code))

#define SERIAL_PORT  0x10000000
#define RTC_ADDR     0x0200BFF8