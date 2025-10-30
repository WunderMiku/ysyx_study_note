#ifndef __NPC_H__
#define __NPC_H__

#include "verilatedos.h"
#include <stdint.h>

#define COLOR_RED   "\033[1;31m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_NONE  "\033[0m"

#define MEM_BASE 0x80000000
#define MEM_SIZE 0x8000000

#define SERIAL_PORT  0x10000000
#define RTC_ADDR     0x10000060

extern vluint32_t* M;
int exec(int n);

#endif // NPC_H__