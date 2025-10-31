#ifndef __NPC_H__
#define __NPC_H__

#include "Vtop.h"
#include "verilatedos.h"
#include <stdint.h>

#define COLOR_RED   "\033[1;31m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_NONE  "\033[0m"

#define MEM_BASE 0x80000000
#define MEM_SIZE 0x8000000

#define SERIAL_PORT  0x10000000
#define RTC_ADDR     0x10000060

typedef enum {
	NPC_RUNNING = 0,
	NPC_END,
	NPC_QUIT,
	NPC_ABORT
} NpcStateType;

typedef struct {
  NpcStateType state;
} NpcState;

extern NpcState npcState;
extern std::unique_ptr<Vtop> dut;

extern vluint32_t* M;
void cpuExec(uint32_t n);

#endif // NPC_H__